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

		TimeoutDuration = SavedSourceSettings->TimeoutDuration;
		bApplyObjectHeight = SavedSourceSettings->bApplyObjectHeight;
		bApplyObjectScale = SavedSourceSettings->bApplyObjectScale;
		bOffsetObjectPositionOnCentroid = SavedSourceSettings->bOffsetObjectPositionOnCentroid;
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
			TimeoutDuration = SavedSourceSettings->TimeoutDuration;
			bApplyObjectHeight = SavedSourceSettings->bApplyObjectHeight;
			bApplyObjectScale = SavedSourceSettings->bApplyObjectScale;
			bOffsetObjectPositionOnCentroid = SavedSourceSettings->bOffsetObjectPositionOnCentroid;
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

	SourceStatus = LOCTEXT("SourceStatus_Receiving", "Receiving");

	ThreadName = "LiveLinkAugmenta Receiver ";
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
						//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received Augmenta message size %d"), ReceivedDataSize);

						HandleOSCPacket(OSCPP::Server::Packet(ReceiveBuffer.GetData(), ReceivedDataSize));
					}
				}
			}

			//Remove inactive objects
			RemoveInactiveObjects();
		}
	}
	
	return 0;
}

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

FLiveLinkAugmentaVideoOutput FLiveLinkAugmentaSource::GetAugmentaVideoOutput()
{
	return AugmentaVideoOutput;
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


void FLiveLinkAugmentaSource::HandleOSCPacket(const OSCPP::Server::Packet& Packet)
{
	FLiveLinkAugmentaObject CurrentAugmentaObject;

	if (Packet.isBundle()) {

		UE_LOG(LogLiveLinkAugmenta, Warning, TEXT("LiveLinkAugmentaSource: Received OSC bundle. This should not happen in Augmenta protocol V2."));

	} else {

		// Convert to message
		OSCPP::Server::Message msg(Packet);

		// Get argument stream
		OSCPP::Server::ArgStream args(msg.args());

		const char* address = msg.address();

		//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received OSC message %s."), UTF8_TO_TCHAR(address));

		if (msg == "/scene") {

			//Update scene object
			AugmentaScene.Frame = args.int32();
			AugmentaScene.ObjectCount = args.int32();
			AugmentaScene.Size.X = args.float32();
			AugmentaScene.Size.Y = args.float32();

			AugmentaScene.Position = FVector::ZeroVector;

			AugmentaScene.Rotation = FQuat::Identity;

			AugmentaScene.Scale.X = AugmentaScene.Size.Y;
			AugmentaScene.Scale.Y = AugmentaScene.Size.X;
			AugmentaScene.Scale.Z = 1;

			if (!bDisableSubjectsUpdate) {
				//Update scene subject
				FLiveLinkFrameDataStruct SceneFrameData(FLiveLinkTransformFrameData::StaticStruct());
				FLiveLinkTransformFrameData* SceneTransformFrameData = SceneFrameData.Cast<FLiveLinkTransformFrameData>();

				SceneTransformFrameData->Transform = FTransform(AugmentaScene.Rotation, AugmentaScene.Position, AugmentaScene.Scale);

				FName CurrentName = FName(SceneName.ToString() + "_Scene");
				Send(&SceneFrameData, CurrentName);
			}

			//Send scene updated event
			if (OnLiveLinkAugmentaSceneUpdated.IsBound())
			{
				OnLiveLinkAugmentaSceneUpdated.Execute(AugmentaScene);
			}

		} else if (msg == "/fusion") {

			//Update video output object
			AugmentaVideoOutput.Offset.X = args.float32() * MetersToUnrealUnits;
			AugmentaVideoOutput.Offset.Y = args.float32() * MetersToUnrealUnits;
			AugmentaVideoOutput.Size.X = args.float32();
			AugmentaVideoOutput.Size.Y = args.float32();
			AugmentaVideoOutput.Resolution.X = args.int32();
			AugmentaVideoOutput.Resolution.Y = args.int32();

			AugmentaVideoOutput.Position.X = (AugmentaScene.Position.X + AugmentaScene.Size.Y * .5f * MetersToUnrealUnits) - AugmentaVideoOutput.Offset.Y - AugmentaVideoOutput.Size.Y * .5f * MetersToUnrealUnits;
			AugmentaVideoOutput.Position.Y = (AugmentaScene.Position.Y - AugmentaScene.Size.X * .5f * MetersToUnrealUnits) + AugmentaVideoOutput.Offset.X + AugmentaVideoOutput.Size.X * .5f * MetersToUnrealUnits;
			AugmentaVideoOutput.Position.Z = AugmentaScene.Position.Z;

			AugmentaVideoOutput.Rotation = AugmentaScene.Rotation;

			AugmentaVideoOutput.Scale.X = AugmentaVideoOutput.Size.Y;
			AugmentaVideoOutput.Scale.Y = AugmentaVideoOutput.Size.X;
			AugmentaVideoOutput.Scale.Z = 1;

			if (!bDisableSubjectsUpdate) {
				//Update video output subject
				FLiveLinkFrameDataStruct VideoOutputFrameData(FLiveLinkTransformFrameData::StaticStruct());
				FLiveLinkTransformFrameData* VideoOutputTransformFrameData = VideoOutputFrameData.Cast<FLiveLinkTransformFrameData>();

				VideoOutputTransformFrameData->Transform = FTransform(AugmentaVideoOutput.Rotation, AugmentaVideoOutput.Position, AugmentaVideoOutput.Scale);

				FName CurrentName = FName(SceneName.ToString() + "_VideoOutput");
				Send(&VideoOutputFrameData, CurrentName);
			}

			//Send video output updated event
			if (OnLiveLinkAugmentaVideoOutputUpdated.IsBound())
			{
				OnLiveLinkAugmentaVideoOutputUpdated.Execute(AugmentaVideoOutput);
			}

		} else if (msg == "/object/enter") {

			//Create augmenta object
			ReadAugmentaObjectFromOSC(&CurrentAugmentaObject, &args);

			if (!AugmentaObjects.Contains(CurrentAugmentaObject.Id)) {
				AddAugmentaObject(CurrentAugmentaObject);
			}

		} else if (msg == "/object/update") {

			//Update augmenta object
			ReadAugmentaObjectFromOSC(&CurrentAugmentaObject, &args);

			if (AugmentaObjects.Contains(CurrentAugmentaObject.Id)) {
				UpdateAugmentaObject(CurrentAugmentaObject);
			}
			else {
				AddAugmentaObject(CurrentAugmentaObject);
			}
		
		} else if (msg == "/object/leave") {

			//Remove augmenta object
			ReadAugmentaObjectFromOSC(&CurrentAugmentaObject, &args);

			if (AugmentaObjects.Contains(CurrentAugmentaObject.Id)) {
				RemoveAugmentaObject(CurrentAugmentaObject);
			}
		
		} else if (msg == "/object/enter/extra") {
		
			UpdateAugmentaObjectExtraFromOSC(&args);

		} else if (msg == "/object/update/extra") {
			
			UpdateAugmentaObjectExtraFromOSC(&args);

		} else if (msg == "/object/leave/extra") {

			UpdateAugmentaObjectExtraFromOSC(&args);
		
		} else {
			// Simply print unknown messages
			UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received unknown OSC message."));
		}
	}
}

void FLiveLinkAugmentaSource::ReadAugmentaObjectFromOSC(FLiveLinkAugmentaObject* AugmentaObject, OSCPP::Server::ArgStream* Args) {

	AugmentaObject->Frame = Args->int32();
	AugmentaObject->Id = Args->int32();
	AugmentaObject->Oid = Args->int32();
	AugmentaObject->Age = Args->float32();
	AugmentaObject->Centroid.X = Args->float32();
	AugmentaObject->Centroid.Y = Args->float32();
	AugmentaObject->Velocity.X = Args->float32();
	AugmentaObject->Velocity.Y = Args->float32();
	AugmentaObject->Orientation = Args->float32();
	AugmentaObject->BoundingRectPos.X = Args->float32();
	AugmentaObject->BoundingRectPos.Y = Args->float32();
	AugmentaObject->BoundingRectSize.X = Args->float32();
	AugmentaObject->BoundingRectSize.Y = Args->float32();
	AugmentaObject->BoundingRectRotation = Args->float32();
	AugmentaObject->Height = Args->float32();

	if (bApplyObjectScale && !bOffsetObjectPositionOnCentroid) {
		AugmentaObject->Position.X = (.5f - AugmentaObject->BoundingRectPos.Y) * AugmentaScene.Size.Y * MetersToUnrealUnits;
		AugmentaObject->Position.Y = (AugmentaObject->BoundingRectPos.X - .5f) * AugmentaScene.Size.X * MetersToUnrealUnits;
	}
	else {
		AugmentaObject->Position.X = (.5f - AugmentaObject->Centroid.Y) * AugmentaScene.Size.Y * MetersToUnrealUnits;
		AugmentaObject->Position.Y = (AugmentaObject->Centroid.X - .5f) * AugmentaScene.Size.X * MetersToUnrealUnits;
	}

	AugmentaObject->Position.Z = bApplyObjectHeight ? AugmentaObject->Height * .5f * MetersToUnrealUnits : 0;

	AugmentaObject->Position += AugmentaScene.Position;

	AugmentaObject->Rotation = FQuat(FRotator(0, -AugmentaObject->BoundingRectRotation, 0));

	if (bApplyObjectScale) {
		AugmentaObject->Scale.X = AugmentaObject->BoundingRectSize.Y * AugmentaScene.Size.Y;
		AugmentaObject->Scale.Y = AugmentaObject->BoundingRectSize.X * AugmentaScene.Size.X;
		AugmentaObject->Scale.Z = AugmentaObject->Height;
	}
	else {
		AugmentaObject->Scale = FVector::OneVector;
	}

	AugmentaObject->LastUpdateTime = FDateTime::Now();
}

void FLiveLinkAugmentaSource::UpdateAugmentaObjectExtraFromOSC(OSCPP::Server::ArgStream* Args) {

	const int Frame = Args->int32();
	const int Id = Args->int32();
	const int Oid = Args->int32();

	if (AugmentaObjects.Contains(Id)) {
		AugmentaObjects[Id].Highest.X = Args->float32();
		AugmentaObjects[Id].Highest.Y = Args->float32();
		AugmentaObjects[Id].Distance = Args->float32();
		AugmentaObjects[Id].Reflectivity = Args->float32();
	}
}

void FLiveLinkAugmentaSource::AddAugmentaObject(FLiveLinkAugmentaObject AugmentaObject)
{
	//Create new object
	AugmentaObjects.Emplace(AugmentaObject.Id, AugmentaObject);

	//Update augmenta object subject
	UpdateAugmentaObjectSubject(AugmentaObject);

	//Send object entered event
	if (OnLiveLinkAugmentaObjectEntered.IsBound())
	{
		OnLiveLinkAugmentaObjectEntered.Execute(AugmentaObject);
	}
}

void FLiveLinkAugmentaSource::UpdateAugmentaObject(FLiveLinkAugmentaObject AugmentaObject)
{
	//Update existing object
	AugmentaObjects[AugmentaObject.Id] = AugmentaObject;

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

void FLiveLinkAugmentaSource::RemoveAugmentaObject(FLiveLinkAugmentaObject AugmentaObject)
{
	if (!bDisableSubjectsUpdate) {
		FName CurrentName = FName(SceneName.ToString() + "_Object_" + FString::FromInt(AugmentaObject.Oid));

		if (EncounteredSubjects.Contains(CurrentName)) {
			EncounteredSubjects.Remove(CurrentName);
			Client->RemoveSubject_AnyThread({ SourceGuid, CurrentName });
		}
	}

	//Send object will leave event
	if (OnLiveLinkAugmentaObjectWillLeave.IsBound())
	{
		OnLiveLinkAugmentaObjectWillLeave.Execute(AugmentaObject);
	}

	AugmentaObjects.Remove(AugmentaObject.Id);
}

void FLiveLinkAugmentaSource::UpdateAugmentaObjectSubject(FLiveLinkAugmentaObject AugmentaObject)
{
	//Update augmenta object subject
	FLiveLinkFrameDataStruct ObjectFrameData(FLiveLinkTransformFrameData::StaticStruct());
	FLiveLinkTransformFrameData* ObjectTransformFrameData = ObjectFrameData.Cast<FLiveLinkTransformFrameData>();

	ObjectTransformFrameData->Transform = FTransform(AugmentaObject.Rotation, AugmentaObject.Position, AugmentaObject.Scale);

	FName CurrentName = FName(SceneName.ToString() + "_Object_" + FString::FromInt(AugmentaObject.Oid));
	Send(&ObjectFrameData, CurrentName);
}

void FLiveLinkAugmentaSource::RemoveInactiveObjects()
{
	FDateTime CurrentTime = FDateTime::Now();

	ObjectsToRemove.Empty();

	for (auto& Elem : AugmentaObjects) {
		if ((CurrentTime - Elem.Value.LastUpdateTime).GetTotalSeconds() > TimeoutDuration) {
			ObjectsToRemove.Emplace(Elem.Value.Id);
		}
	}

	for (auto& id : ObjectsToRemove) {
		RemoveAugmentaObject(AugmentaObjects[id]);
	}
}

#undef LOCTEXT_NAMESPACE
