// Copyright Epic Games, Inc. All Rights Reserved.

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
	SourceMachineName = FText::Format(LOCTEXT("AugmentaSourceMachineName", "{0}:{1}"), FText::FromString(ConnectionSettings.IPAddress), FText::AsNumber(ConnectionSettings.UDPPortNumber, &FNumberFormattingOptions::DefaultNoGrouping()));

	FIPv4Address::Parse(ConnectionSettings.IPAddress, DeviceEndpoint.Address);
	DeviceEndpoint.Port = ConnectionSettings.UDPPortNumber;

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

	UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Closed scene %s with IP address %s"), *SceneName, *DeviceEndpoint.ToString());
}

void FLiveLinkAugmentaSource::ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid)
{
	Client = InClient;
	SourceGuid = InSourceGuid;
}

void FLiveLinkAugmentaSource::InitializeSettings(ULiveLinkSourceSettings* Settings) {

	ULiveLinkAugmentaSourceSettings* AugmentaSourceSettings = Cast<ULiveLinkAugmentaSourceSettings>(Settings);

	if (AugmentaSourceSettings != nullptr) {
		AugmentaSourceSettings->SceneName = SceneName;
		AugmentaSourceSettings->SourceReference = this;
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
	const float MetersToUnrealUnits = 100.0f;
	const int ObjectDataSize = 12;
	const int SceneDataSize = 12;
	FLiveLinkAugmentaObject AugmentaObject;

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
						UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received Augmenta message size %d"), ReceivedDataSize);

						/* TEST WITH CUSTOM POINT 3D FROM CHATAIGNE UDP 
						if (ReceivedDataSize >= SceneDataSize) {

							//Clean desired subjects
							DesiredSubjects.Empty();

							//In this test the first 3 floats (3x4bytes) are the scene position
							float PositionX = *(float*)(&ReceiveBuffer.GetData()[0]);
							AugmentaScene.Position.X = PositionX * MetersToUnrealUnits;;
							//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received Augmenta message %f"), ScenePositionX);

							float PositionY = *(float*)(&ReceiveBuffer.GetData()[4]);
							AugmentaScene.Position.Y = PositionY * MetersToUnrealUnits;
							//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received Augmenta message %f"), ScenePositionY);

							float PositionZ = *(float*)(&ReceiveBuffer.GetData()[8]);
							AugmentaScene.Position.Z = PositionZ * MetersToUnrealUnits;
							//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Received Augmenta message %f"), ScenePositionZ);

							FLiveLinkFrameDataStruct SceneFrameData(FLiveLinkTransformFrameData::StaticStruct());
							FLiveLinkTransformFrameData* SceneTransformFrameData = SceneFrameData.Cast<FLiveLinkTransformFrameData>();

							SceneTransformFrameData->Transform = FTransform(AugmentaScene.Rotation, AugmentaScene.Position);

							//Send scene updated event
							if (OnLiveLinkAugmentaSceneUpdated.IsBound())
							{
								OnLiveLinkAugmentaSceneUpdated.Execute(AugmentaScene);
							}

							FName CurrentName = FName(SceneName + "_Scene");
							Send(&SceneFrameData, CurrentName);
							DesiredSubjects.Emplace(CurrentName);

							//Next floats are object positions
							int ObjectCount = (ReceivedDataSize - SceneDataSize) / ObjectDataSize;
							//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Augmenta objects count is %d"), ObjectCount);

							for (int i = 0; i < ObjectCount; i++) {

								FLiveLinkFrameDataStruct ObjectFrameData(FLiveLinkTransformFrameData::StaticStruct());
								FLiveLinkTransformFrameData* ObjectTransformFrameData = ObjectFrameData.Cast<FLiveLinkTransformFrameData>();

								ObjectTransformFrameData->MetaData.StringMetaData.Empty();

								//ObjectTransformFrameData->MetaData.StringMetaData.Emplace(FName(TEXT("Id")), FString::Printf(TEXT("%d"), i));

								AugmentaObject.Id = i;

								PositionX = *(float*)(&ReceiveBuffer.GetData()[(i * ObjectDataSize) + SceneDataSize]);
								PositionY = *(float*)(&ReceiveBuffer.GetData()[(i * ObjectDataSize) + SceneDataSize + 4]);
								PositionZ = *(float*)(&ReceiveBuffer.GetData()[(i * ObjectDataSize) + SceneDataSize + 8]);

								ObjectTransformFrameData->Transform = FTransform(FQuat(1, 0, 0, 0), FVector(PositionX * MetersToUnrealUnits, PositionY * MetersToUnrealUnits, PositionZ * MetersToUnrealUnits));

								//ObjectTransformFrameData->MetaData.StringMetaData.Emplace(FName(TEXT("PositionX")), FString::Printf(TEXT("%f"), PositionX * MetersToUnrealUnits));
								//ObjectTransformFrameData->MetaData.StringMetaData.Emplace(FName(TEXT("PositionY")), FString::Printf(TEXT("%f"), PositionY * MetersToUnrealUnits));
								//ObjectTransformFrameData->MetaData.StringMetaData.Emplace(FName(TEXT("PositionZ")), FString::Printf(TEXT("%f"), PositionZ * MetersToUnrealUnits));

								AugmentaObject.Position.X = PositionX * MetersToUnrealUnits;
								AugmentaObject.Position.Y = PositionY * MetersToUnrealUnits;
								AugmentaObject.Position.Z = PositionZ * MetersToUnrealUnits;

								if (AugmentaObjects.Contains(AugmentaObject.Id)) {
									//Update existing object
									AugmentaObjects[AugmentaObject.Id] = AugmentaObject;

									//Send object updated event
									if (OnLiveLinkAugmentaObjectUpdated.IsBound())
									{
										OnLiveLinkAugmentaObjectUpdated.Execute(AugmentaObject);
									}
								}
								else {
									//Create new object
									AugmentaObjects.Emplace(AugmentaObject.Id, AugmentaObject);

									//Send object entered event
									if (OnLiveLinkAugmentaObjectEntered.IsBound())
									{
										OnLiveLinkAugmentaObjectEntered.Execute(AugmentaObject);
									}
								}

								CurrentName = FName(SceneName + "_Object_" + FString::FromInt(i));
								Send(&ObjectFrameData, CurrentName);
								DesiredSubjects.Emplace(CurrentName);

							}

							//Removed undesired subjects
							UndesiredSubjects.Empty();

							//Get all undesired subjects (subjects that are not in the current received message)
							for (auto& Elem : EncounteredSubjects) {
								//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Current Encountered Subject in loop is %s"), *Elem.ToString());
								if (!DesiredSubjects.Contains(Elem)) {
									UndesiredSubjects.Add(Elem);
								}
							}

							//Remove undesired subjects
							for (auto& Elem : UndesiredSubjects) {
								//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Current Undesired Subject in loop is %s"), *Elem.ToString());
								EncounteredSubjects.Remove(Elem);
								Client->RemoveSubject_AnyThread({ SourceGuid, Elem });

								//Remove object from the list
								FString SIdToRemove = Elem.ToString();
								SIdToRemove.RemoveFromStart(SceneName + "_Object_");
								int IdToRemove = FCString::Atoi(*SIdToRemove);

								//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: IdToRemove is %s : %d"), *SIdToRemove, IdToRemove);

								if (AugmentaObjects.Contains(IdToRemove)) {

									//Send object will leave event
									if (OnLiveLinkAugmentaObjectWillLeave.IsBound())
									{
										OnLiveLinkAugmentaObjectWillLeave.Execute(AugmentaObjects[IdToRemove]);
									}

									AugmentaObjects.Remove(IdToRemove);
								}
							}

							//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaSource: Current object count is %d"), AugmentaObjects.Num());
						}
						*/
					}
				}
			}
		}
	}
	
	return 0;
}

FString FLiveLinkAugmentaSource::GetSceneName()
{
	return SceneName;
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
		Client->PushSubjectStaticData_AnyThread({ SourceGuid, SubjectName }, ULiveLinkTransformRole::StaticClass(), MoveTemp(StaticData));

		EncounteredSubjects.Add(SubjectName);
	}

	Client->PushSubjectFrameData_AnyThread({ SourceGuid, SubjectName }, MoveTemp(*FrameDataToSend));
}

#undef LOCTEXT_NAMESPACE
