// Copyright Augmenta 2023, All Rights Reserved.

#include "LiveLinkAugmentaSource.h"
#include "LiveLinkAugmenta.h"
#include "ILiveLinkClient.h"
#include "Engine/Engine.h"
#include "Async/Async.h"
#include "LiveLinkAugmentaSourceSettings.h"
#include "LiveLinkAugmentaData.h"
#include "Misc/CoreDelegates.h"
#include "Roles/LiveLinkTransformRole.h"

#include "Common/UdpSocketBuilder.h"

#define LOCTEXT_NAMESPACE "LiveLinkAugmentaSourceFactory"

FLiveLinkAugmentaSource::FLiveLinkAugmentaSource(const FLiveLinkAugmentaConnectionSettings& ConnectionSettings)
: Client(nullptr)
, Stopping(false)
, Thread(nullptr)
, LocalUpdateRateInHz(ConnectionSettings.LocalUpdateRateInHz)
, SceneName(ConnectionSettings.SceneName)
{
	SourceStatus = LOCTEXT("SourceStatus_NoData", "No data");
	SourceType = LOCTEXT("SourceType_Augmenta", "Augmenta");
	SourceMachineName = FText::Format(LOCTEXT("AugmentaSourceMachineName", "{0}:{1}"), FText::FromString(ConnectionSettings.IPAddress), FText::AsNumber(ConnectionSettings.PortNumber, &FNumberFormattingOptions::DefaultNoGrouping()));

	FIPv4Address::Parse(ConnectionSettings.IPAddress, DeviceEndpoint.Address);
	DeviceEndpoint.Port = ConnectionSettings.PortNumber;

	Socket = FUdpSocketBuilder(TEXT("AugmentaListenerSocket"))
		.AsNonBlocking()
		.AsReusable()
		.BoundToEndpoint(DeviceEndpoint)
		.WithReceiveBufferSize(ReceiveBufferSize);

	if ((Socket != nullptr) && (Socket->GetSocketType() == SOCKTYPE_Datagram))
	{
		ReceiveBuffer.SetNumUninitialized(ReceiveBufferSize);
		SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
		DeferredStartDelegateHandle = FCoreDelegates::OnEndFrame.AddRaw(this, &FLiveLinkAugmentaSource::Start);

		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Opened UDP socket with IP address %s"), *DeviceEndpoint.ToString());
	}
	else
	{
		UE_LOG(LogLiveLinkAugmenta, Error, TEXT("LiveLinkAugmentaSource: Failed to open UDP socket with IP address %s"), *DeviceEndpoint.ToString());
	}
}

FLiveLinkAugmentaSource::~FLiveLinkAugmentaSource()
{
	// This could happen if the object is destroyed before FCoreDelegates::OnEndFrame calls FLiveLinkAugmentaSource::Start
	if (DeferredStartDelegateHandle.IsValid())
	{
		FCoreDelegates::OnEndFrame.Remove(DeferredStartDelegateHandle);
	}

	Stop();

	if (Thread != nullptr)
	{
		Thread->WaitForCompletion();
		delete Thread;
		Thread = nullptr;
	}

	if (Socket != nullptr)
	{
		Socket->Close();
		SocketSubsystem->DestroySocket(Socket);
		Socket = nullptr;
	}

	if(OnLiveLinkAugmentaSourceDestroyed.IsBound())
	{
		OnLiveLinkAugmentaSourceDestroyed.Execute();
	}

	UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Closed scene %s with IP address %s"), *SceneName.ToString(), *DeviceEndpoint.ToString());
}

void FLiveLinkAugmentaSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;
}

void FLiveLinkAugmentaSource::InitializeSettings(ULiveLinkSourceSettings* Settings) {

	// Save our source settings pointer so we can use it directly
	SavedSourceSettings = Cast<ULiveLinkAugmentaSourceSettings>(Settings);

	if (SavedSourceSettings != nullptr) {
		SavedSourceSettings->SceneName = SceneName;
		SavedSourceSettings->SourceReference = this;

		bApplyObjectHeight = SavedSourceSettings->bApplyObjectHeight;
		bUseBoundingBox = SavedSourceSettings->bUseBoundingBox;
		bDisableSubjectsUpdate = SavedSourceSettings->bDisableSubjectsUpdate;
	}
}

void FLiveLinkAugmentaSource::OnSettingsChanged(ULiveLinkSourceSettings* Settings, const FPropertyChangedEvent& PropertyChangedEvent)
{
	ILiveLinkSource::OnSettingsChanged(Settings, PropertyChangedEvent);

	FProperty* MemberProperty = PropertyChangedEvent.MemberProperty;
	FProperty* Property = PropertyChangedEvent.Property;
	if (Property && MemberProperty && (PropertyChangedEvent.ChangeType != EPropertyChangeType::Interactive))
	{
		ULiveLinkAugmentaSourceSettings* SourceSettings = Cast<ULiveLinkAugmentaSourceSettings>(Settings);
		if (SavedSourceSettings != SourceSettings)
		{
			UE_LOG(LogLiveLinkAugmenta, Error, TEXT("LiveLinkAugmentaSource: OnSettingsChanged pointers do not match. This should never happen. Try recreating your source."));
			return;
		}

		if (SourceSettings != nullptr)
		{
			bApplyObjectHeight = SavedSourceSettings->bApplyObjectHeight;
			bUseBoundingBox = SavedSourceSettings->bUseBoundingBox;
			bDisableSubjectsUpdate = SavedSourceSettings->bDisableSubjectsUpdate;
		}
	}
}

bool FLiveLinkAugmentaSource::IsSourceStillValid() const
{
	// Source is valid if we have a valid thread
	bool bIsSourceValid = !Stopping && (Thread != nullptr);
	return bIsSourceValid;
}

bool FLiveLinkAugmentaSource::RequestSourceShutdown()
{
	Stop();

	return true;
}

// FRunnable interface
void FLiveLinkAugmentaSource::Start()
{
	check(DeferredStartDelegateHandle.IsValid());

	FCoreDelegates::OnEndFrame.Remove(DeferredStartDelegateHandle);
	DeferredStartDelegateHandle.Reset();

	SourceStatus = LOCTEXT("SourceStatus_Listening", "Listening");

	ThreadName = "LiveLinkAugmenta Receiver";
	ThreadName.AppendInt(FAsyncThreadIndex::GetNext());

	Thread = FRunnableThread::Create(this, *ThreadName, 128 * 1024, TPri_AboveNormal, FPlatformAffinity::GetPoolThreadMask());
}

void FLiveLinkAugmentaSource::Stop()
{
	Stopping = true;
}

uint32 FLiveLinkAugmentaSource::Run()
{
	const double SleepDeltaTime = 1.0 / (double)LocalUpdateRateInHz;

	while (!Stopping)
	{
		if (Socket && Socket->Wait(ESocketWaitConditions::WaitForRead, SleepDeltaTime))
		{
			uint32 PendingDataSize = 0;
			while (Socket && Socket->HasPendingData(PendingDataSize))
			{
				int32 ReceivedDataSize = 0;
				if (Socket && Socket->Recv(ReceiveBuffer.GetData(), ReceiveBufferSize, ReceivedDataSize))
				{
					if (ReceivedDataSize > 0)
					{
						SourceStatus = LOCTEXT("SourceStatus_Receiving", "Receiving");
						LastReceivedMessageTime = FDateTime::Now();

						HandleReceivedMessage(OSCPP::Server::Packet(ReceiveBuffer.GetData(), ReceivedDataSize));
					}
				}
			}
		}

		if ((FDateTime::Now() - LastReceivedMessageTime).GetTotalSeconds() > ReceivingStatusTimeout)
		{
			SourceStatus = LOCTEXT("SourceStatus_Listening", "Listening");
		}
	}
	
	return 0;
}

void FLiveLinkAugmentaSource::Send(FLiveLinkFrameDataStruct* FrameDataToSend, FName SubjectName)
{

	if (Stopping || (Client == nullptr))
	{
		return;
	}

	if (!EncounteredSubjects.Contains(SubjectName))
	{
		FLiveLinkStaticDataStruct StaticData(FLiveLinkTransformStaticData::StaticStruct());
		StaticData.Cast<FLiveLinkTransformStaticData>()->bIsScaleSupported = true;
		Client->PushSubjectStaticData_AnyThread({ SourceGuid, SubjectName }, ULiveLinkTransformRole::StaticClass(), MoveTemp(StaticData));

		EncounteredSubjects.Add(SubjectName);
	}

	Client->PushSubjectFrameData_AnyThread({ SourceGuid, SubjectName }, MoveTemp(*FrameDataToSend));
}

/********************/
/**** ACCESSORS *****/
/********************/

FName FLiveLinkAugmentaSource::GetSceneName()
{
	return SceneName;
}

FLiveLinkAugmentaScene FLiveLinkAugmentaSource::GetAugmentaScene()
{
	return AugmentaScene;
}

TMap<int, FLiveLinkAugmentaObject> FLiveLinkAugmentaSource::GetAugmentaObjects()
{
	return AugmentaObjects;
}

bool FLiveLinkAugmentaSource::GetAugmentaObjectById(FLiveLinkAugmentaObject& AugmentaObject, int Id)
{
	if(AugmentaObjects.Contains(Id))
	{
		AugmentaObject = AugmentaObjects[Id];
	}
	return false;
}

int FLiveLinkAugmentaSource::GetAugmentaObjectsCount()
{
	return AugmentaObjects.Num();
}

bool FLiveLinkAugmentaSource::ContainsId(int Id)
{
	return AugmentaObjects.Contains(Id);
}

/**********************/
/**** OSC PARSING *****/
/**********************/

void FLiveLinkAugmentaSource::HandleReceivedMessage(const OSCPP::Server::Packet& Packet)
{
	FLiveLinkAugmentaObject CurrentAugmentaObject;

	if (Packet.isBundle()) {

		// Convert to bundle
		OSCPP::Server::Bundle Bundle(Packet);

		// Get packet stream
		OSCPP::Server::PacketStream Packets(Bundle.packets());

		/////Check bundle type
		const OSCPP::Server::Message Message(Packets.next());
		const FString Address = Message.address();
		TArray<FString> AddressArgs;
		Address.ParseIntoArray(AddressArgs, TEXT("/"), true);

		if (IsAugmentaMessageValid(AddressArgs))
		{
			if (AddressArgs[1] == "scene")
			{
				//Parse scene bundle
				ParseSceneBundle(Bundle);
			}
			else if (AddressArgs[1] == "frame")
			{
				//Parse objects bundle
				ParseObjectsBundle(Bundle);
			}
		}
		else
		{
			SourceStatus = LOCTEXT("SourceStatus_IncorrectProtocol", "Incorrect Protocol Received");
		}

	} else {

		SourceStatus = LOCTEXT("SourceStatus_IncorrectProtocol", "Incorrect Protocol Received");
	}
}

void FLiveLinkAugmentaSource::ParseSceneBundle(const OSCPP::Server::Bundle& Bundle)
{
	// Get packet stream
	OSCPP::Server::PacketStream Packets(Bundle.packets());

	// Iterate over all the packets and call ParseScenePacket recursively.
	// Caution: Might lead to stack overflow!
	while (!Packets.atEnd()) {
		ParseScenePacket(Packets.next());
	}

	if (!bDisableSubjectsUpdate) {
		//Update scene subject
		FLiveLinkFrameDataStruct SceneFrameData(FLiveLinkTransformFrameData::StaticStruct());
		FLiveLinkTransformFrameData* SceneTransformFrameData = SceneFrameData.Cast<FLiveLinkTransformFrameData>();

		SceneTransformFrameData->Transform = FTransform(FQuat::Identity, AugmentaScene.Position, AugmentaScene.Size);

		FName CurrentName = FName(SceneName.ToString() + "_Scene");
		Send(&SceneFrameData, CurrentName);
	}

	//Send scene updated event
	if (OnLiveLinkAugmentaSceneUpdated.IsBound())
	{
		OnLiveLinkAugmentaSceneUpdated.Execute(AugmentaScene);
	}
}

void FLiveLinkAugmentaSource::ParseScenePacket(const OSCPP::Server::Packet& Packet)
{
	// Convert to message
	const OSCPP::Server::Message Message(Packet);

	// Get argument stream
	OSCPP::Server::ArgStream Args(Message.args());

	const FString Address = Message.address();

	TArray<FString> AddressArgs;
	Address.ParseIntoArray(AddressArgs, TEXT("/"), true);

	//Ensure address is correct
	if (IsAugmentaMessageValid(AddressArgs))
	{
		if (AddressArgs[2] == "clear")
		{
			RemoveAllObjects();
		}
		else if (AddressArgs[2] == "pos")
		{
			AugmentaScene.Position.X = Args.float32();
			AugmentaScene.Position.Y = Args.float32();
			AugmentaScene.Position.Z = Args.float32();
		}
		else if (AddressArgs[2] == "size")
		{
			AugmentaScene.Size.X = Args.float32();
			AugmentaScene.Size.Y = Args.float32();
			AugmentaScene.Size.Z = Args.float32();
		}
		else if (AddressArgs[2] == "video" && AddressArgs.Num() >= 4)
		{
			if (AddressArgs[3] == "pos")
			{
				AugmentaScene.VideoPosition.X = Args.float32();
				AugmentaScene.VideoPosition.Y = Args.float32();
				AugmentaScene.VideoPosition.Z = Args.float32();
			}
			else if (AddressArgs[3] == "size")
			{
				AugmentaScene.VideoSize.X = Args.float32();
				AugmentaScene.VideoSize.Y = Args.float32();
				AugmentaScene.VideoSize.Z = Args.float32();
			}
			else if (AddressArgs[3] == "res")
			{
				AugmentaScene.VideoResolution.X = Args.int32();
				AugmentaScene.VideoResolution.Y = Args.int32();
			}
		}
	}
	else
	{
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received unknown OSC message with address %s."), *Address);
	}
}

void FLiveLinkAugmentaSource::ParseObjectsBundle(const OSCPP::Server::Bundle& Bundle)
{
	// Remove objects that left
	RemoveObjectsThatLeft(Bundle);

	// Clear new objects list to use it to record updated objects ids
	ReceivedObjectsIds.Empty();

	// Parse all packets
	OSCPP::Server::PacketStream Packets(Bundle.packets());

	// Iterate over all the packets and call ParseObjectsPacket recursively.
	// Caution: Might lead to stack overflow!
	while (!Packets.atEnd()) {
		ParseObjectsPacket(Packets.next());
	}

	//Compare current object list and received one and send corresponding Augmenta events
	for (const auto& Id : ReceivedObjectsIds) {
		if(CurrentObjectsIds.Contains(Id))
		{
			//Updated object
			UpdateAugmentaObject(AugmentaObjects[Id]);

		} else
		{
			//Created object
			CurrentObjectsIds.Add(Id);
			AddAugmentaObject(AugmentaObjects[Id]);
		}
	}
}

void FLiveLinkAugmentaSource::ParseObjectsPacket(const OSCPP::Server::Packet& Packet)
{
	// Convert to message
	const OSCPP::Server::Message Message(Packet);

	// Get argument stream
	OSCPP::Server::ArgStream Args(Message.args());

	const FString Address = Message.address();
	TArray<FString> AddressArgs;
	Address.ParseIntoArray(AddressArgs, TEXT("/"), true);

	//Ensure address is correct
	if (IsAugmentaMessageValid(AddressArgs))
	{
		//Every address in the objects bundle is supposed to start by "frame" or an object id
		if (AddressArgs[1] != "frame")
		{
			//Current object id
			const int Id = FCString::Atoi(*AddressArgs[1]);

			//Create Augmenta object if not existing
			//Current object id is not updated at this point, it will be after all parsing is done in ParseObjectsBundle
			if (!AugmentaObjects.Contains(Id))
			{
				FLiveLinkAugmentaObject NewAugmentaObject;
				NewAugmentaObject.Id = Id;

				AugmentaObjects.Emplace(Id, NewAugmentaObject);
			}

			//Add id to list of received Ids
			if (!ReceivedObjectsIds.Contains(Id))
			{
				ReceivedObjectsIds.Add(Id);
			}

			//Check message content
			if (AddressArgs[2] == "uid")
			{
				AugmentaObjects[Id].Uid = Args.int32();
			}
			else if (AddressArgs[2] == "pos")
			{
				AugmentaObjects[Id].CentroidPosition.X = Args.float32();
				AugmentaObjects[Id].CentroidPosition.Y = Args.float32();
				AugmentaObjects[Id].CentroidPosition.Z = Args.float32();

				AugmentaObjects[Id].Position = AugmentaObjects[Id].CentroidPosition;
			}
			else if (AddressArgs[2] == "height")
			{
				AugmentaObjects[Id].Height = Args.float32();

				if (bApplyObjectHeight)
				{
					AugmentaObjects[Id].CentroidPosition.Z += AugmentaObjects[Id].Height * .5f;
					AugmentaObjects[Id].Position.Z += AugmentaObjects[Id].Height * .5f;
				}
			}
			else if (AddressArgs[2] == "weight")
			{
				AugmentaObjects[Id].Confidence = Args.float32();
			}
			else if (AddressArgs[2] == "box" && AddressArgs.Num() >= 4)
			{
				if (AddressArgs[3] == "angle")
				{
					AugmentaObjects[Id].Rotation = FQuat(FRotator(0, Args.float32(), 0));
				}
				else if (AddressArgs[3] == "pos")
				{
					if (bUseBoundingBox) {
						AugmentaObjects[Id].Position.X = Args.float32();
						AugmentaObjects[Id].Position.Y = Args.float32();
						AugmentaObjects[Id].Position.Z = Args.float32();

						if (bApplyObjectHeight)
						{
							AugmentaObjects[Id].Position.Z += AugmentaObjects[Id].Height * .5f;
						}
					}
				}
				else if (AddressArgs[3] == "size")
				{
					if (bUseBoundingBox) {
						AugmentaObjects[Id].Size.X = Args.float32();
						AugmentaObjects[Id].Size.Y = Args.float32();
						AugmentaObjects[Id].Size.Z = Args.float32();
					}
					else
					{
						AugmentaObjects[Id].Size = FVector::OneVector;
					}
				}
			}
		}
	}
	else
	{
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received unknown OSC message with address %s."), *Address);
	}
}

void FLiveLinkAugmentaSource::RemoveObjectsThatLeft(const OSCPP::Server::Bundle& Bundle)
{
	ReceivedObjectsIds.Empty();

	OSCPP::Server::PacketStream Packets(Bundle.packets());

	bool bFoundObjList = false;

	// Iterate over all the packets to find the object list
	while (!bFoundObjList && !Packets.atEnd()) {

		// Convert to message
		const OSCPP::Server::Message Message(Packets.next());

		const FString Address = Message.address();
		TArray<FString> AddressArgs;
		Address.ParseIntoArray(AddressArgs, TEXT("/"), true);

		//Ensure address is correct
		if (IsAugmentaMessageValid(AddressArgs))
		{
			if (AddressArgs[1] == "frame" && AddressArgs[2] == "objlist")
			{
				// Get argument stream
				OSCPP::Server::ArgStream Args(Message.args());

				while (!Args.atEnd())
				{
					ReceivedObjectsIds.Add(Args.int32());
				}

				bFoundObjList = true;
			}
		}
		else
		{
			UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received unknown OSC message with address %s."), *Address);
		}
	}

	// Remove objects that are not in the received objects list
	TArray<int32> IdsToRemove;

	for (const auto& Id : CurrentObjectsIds) {
		if(!ReceivedObjectsIds.Contains(Id))
		{
			IdsToRemove.Add(Id);
		}
	}

	for(const auto& Id:IdsToRemove)
	{
		CurrentObjectsIds.Remove(Id);
		AugmentaObjects.Remove(Id);
		RemoveAugmentaObject(Id);
	}
}

//void FLiveLinkAugmentaSource::ReadAugmentaObjectFromOSC(FLiveLinkAugmentaObject* AugmentaObject, OSCPP::Server::ArgStream* Args) {
//
//	AugmentaObject->Frame = Args->int32();
//	AugmentaObject->Id = Args->int32();
//	AugmentaObject->Oid = Args->int32();
//	AugmentaObject->Age = Args->float32();
//	AugmentaObject->Centroid.X = Args->float32();
//	AugmentaObject->Centroid.Y = Args->float32();
//	AugmentaObject->Velocity.X = Args->float32();
//	AugmentaObject->Velocity.Y = Args->float32();
//	AugmentaObject->Orientation = Args->float32();
//	AugmentaObject->BoundingRectPos.X = Args->float32();
//	AugmentaObject->BoundingRectPos.Y = Args->float32();
//	AugmentaObject->BoundingRectSize.X = Args->float32();
//	AugmentaObject->BoundingRectSize.Y = Args->float32();
//	AugmentaObject->BoundingRectRotation = Args->float32();
//	AugmentaObject->Height = Args->float32();
//
//	if (bApplyObjectScale && !bOffsetObjectPositionOnCentroid) {
//		AugmentaObject->Position.X = (.5f - AugmentaObject->BoundingRectPos.Y) * AugmentaScene.Size.Y * MetersToUnrealUnits;
//		AugmentaObject->Position.Y = (AugmentaObject->BoundingRectPos.X - .5f) * AugmentaScene.Size.X * MetersToUnrealUnits;
//	}
//	else {
//		AugmentaObject->Position.X = (.5f - AugmentaObject->Centroid.Y) * AugmentaScene.Size.Y * MetersToUnrealUnits;
//		AugmentaObject->Position.Y = (AugmentaObject->Centroid.X - .5f) * AugmentaScene.Size.X * MetersToUnrealUnits;
//	}
//
//	AugmentaObject->Position.Z = bApplyObjectHeight ? AugmentaObject->Height * .5f * MetersToUnrealUnits : 0;
//
//	AugmentaObject->Position += AugmentaScene.Position;
//
//	AugmentaObject->Rotation = FQuat(FRotator(0, -AugmentaObject->BoundingRectRotation, 0));
//
//	if (bApplyObjectScale) {
//		AugmentaObject->Scale.X = AugmentaObject->BoundingRectSize.Y * AugmentaScene.Size.Y;
//		AugmentaObject->Scale.Y = AugmentaObject->BoundingRectSize.X * AugmentaScene.Size.X;
//		AugmentaObject->Scale.Z = AugmentaObject->Height;
//	}
//	else {
//		AugmentaObject->Scale = FVector::OneVector;
//	}
//
//	AugmentaObject->LastUpdateTime = FDateTime::Now();
//}

void FLiveLinkAugmentaSource::AddAugmentaObject(FLiveLinkAugmentaObject AugmentaObject)
{
	if (!bDisableSubjectsUpdate) {
		//Update augmenta object subject
		UpdateAugmentaObjectSubject(AugmentaObject);
	}

	//Send object entered event
	if (OnLiveLinkAugmentaObjectEntered.IsBound())
	{
		OnLiveLinkAugmentaObjectEntered.Execute(AugmentaObject);
	}
}

void FLiveLinkAugmentaSource::UpdateAugmentaObject(FLiveLinkAugmentaObject AugmentaObject)
{
	if (!bDisableSubjectsUpdate) {
		//Update augmenta object subject
		UpdateAugmentaObjectSubject(AugmentaObject);
	}

	//Send object updated event
	if (OnLiveLinkAugmentaObjectUpdated.IsBound())
	{
		OnLiveLinkAugmentaObjectUpdated.Execute(AugmentaObject);
	}
}

void FLiveLinkAugmentaSource::RemoveAugmentaObject(int32 Id)
{
	if (!bDisableSubjectsUpdate) {
		FName CurrentName = FName(SceneName.ToString() + "_Object_" + FString::FromInt(Id));

		if (EncounteredSubjects.Contains(CurrentName)) {
			EncounteredSubjects.Remove(CurrentName);
			Client->RemoveSubject_AnyThread({ SourceGuid, CurrentName });
		}
	}

	//Send object left event
	if (OnLiveLinkAugmentaObjectLeft.IsBound())
	{
		OnLiveLinkAugmentaObjectLeft.Execute(Id);
	}
}

void FLiveLinkAugmentaSource::UpdateAugmentaObjectSubject(FLiveLinkAugmentaObject AugmentaObject)
{
	//Update augmenta object subject
	FLiveLinkFrameDataStruct ObjectFrameData(FLiveLinkTransformFrameData::StaticStruct());
	FLiveLinkTransformFrameData* ObjectTransformFrameData = ObjectFrameData.Cast<FLiveLinkTransformFrameData>();

	ObjectTransformFrameData->Transform = FTransform(AugmentaObject.Rotation, AugmentaObject.Position, AugmentaObject.Size);

	const FName CurrentName = FName(SceneName.ToString() + "_Object_" + FString::FromInt(AugmentaObject.Id));
	Send(&ObjectFrameData, CurrentName);
}

void FLiveLinkAugmentaSource::RemoveAllObjects()
{
	for (const auto& Id : CurrentObjectsIds) {
		RemoveAugmentaObject(Id);
	}

	AugmentaObjects.Empty();
	CurrentObjectsIds.Empty();
}

bool FLiveLinkAugmentaSource::IsAugmentaMessageValid(TArray<FString>& AddressArgs)
{
	return (AddressArgs.Num() >= 3 && AddressArgs[0] == "aug");
}

#undef LOCTEXT_NAMESPACE
