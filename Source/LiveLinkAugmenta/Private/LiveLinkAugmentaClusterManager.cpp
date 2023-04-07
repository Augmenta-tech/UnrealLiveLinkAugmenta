// Copyright Augmenta 2023, All Rights Reserved.


#include "LiveLinkAugmentaClusterManager.h"

#include "Cluster/IDisplayClusterClusterManager.h"
#include "DisplayCluster/Public/IDisplayCluster.h"

// Sets default values
ALiveLinkAugmentaClusterManager::ALiveLinkAugmentaClusterManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	AugmentaEventDispatcher = nullptr;
	ClusterManager = nullptr;

	bInitialized = false;
	bUseBinaryClusterEvents = true;
	BinaryEventIdOffset = 0;
}

// Called when the game starts or when spawned
void ALiveLinkAugmentaClusterManager::BeginPlay()
{
	Super::BeginPlay();

	//Get DisplayClusterManager
	ClusterManager = IDisplayCluster::Get().GetClusterMgr();

	//Bind to cluster event listener
	if (ensure(ClusterManager)) {
		ClusterManager->AddClusterEventListener(this);

		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("Augmenta Cluster Manager bound to Cluster Manager successfully."));
	}

	//Bind to Augmenta Manager events
	if(ensure(AugmentaEventDispatcher))
	{
		AugmentaEventDispatcher->OnAugmentaSceneUpdated.AddDynamic(this, &ALiveLinkAugmentaClusterManager::SendSceneUpdatedClusterEvent);
		AugmentaEventDispatcher->OnAugmentaObjectEntered.AddDynamic(this, &ALiveLinkAugmentaClusterManager::SendObjectEnteredClusterEvent);
		AugmentaEventDispatcher->OnAugmentaObjectUpdated.AddDynamic(this, &ALiveLinkAugmentaClusterManager::SendObjectUpdatedClusterEvent);
		AugmentaEventDispatcher->OnAugmentaObjectLeft.AddDynamic(this, &ALiveLinkAugmentaClusterManager::SendObjectLeftClusterEvent);
		AugmentaEventDispatcher->OnAugmentaSourceDestroyed.AddDynamic(this, &ALiveLinkAugmentaClusterManager::SendSourceDestroyedClusterEvent);

		bInitialized = true;

		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("Augmenta Cluster Manager bound to Augmenta Manager successfully."));
	} else
	{
		UE_LOG(LogLiveLinkAugmenta, Error, TEXT("Augmenta Manager reference is invalid in Augmenta Cluster Manager !"));
	}

}

void ALiveLinkAugmentaClusterManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//Unbind from cluster event listener
	if (ClusterManager) {
		ClusterManager->RemoveClusterEventListener(this);
	}

	//Unbind from Augmenta Manager
	if (AugmentaEventDispatcher && bInitialized)
	{
		AugmentaEventDispatcher->OnAugmentaSceneUpdated.RemoveDynamic(this, &ALiveLinkAugmentaClusterManager::SendSceneUpdatedClusterEvent);
		AugmentaEventDispatcher->OnAugmentaObjectEntered.RemoveDynamic(this, &ALiveLinkAugmentaClusterManager::SendObjectEnteredClusterEvent);
		AugmentaEventDispatcher->OnAugmentaObjectUpdated.RemoveDynamic(this, &ALiveLinkAugmentaClusterManager::SendObjectUpdatedClusterEvent);
		AugmentaEventDispatcher->OnAugmentaObjectLeft.RemoveDynamic(this, &ALiveLinkAugmentaClusterManager::SendObjectLeftClusterEvent);
		AugmentaEventDispatcher->OnAugmentaSourceDestroyed.RemoveDynamic(this, &ALiveLinkAugmentaClusterManager::SendSourceDestroyedClusterEvent);
	}

}

void ALiveLinkAugmentaClusterManager::SendSceneUpdatedClusterEvent(const FLiveLinkAugmentaScene AugmentaScene)
{
	if(bUseBinaryClusterEvents)
	{
		FDisplayClusterClusterEventBinary Event;

		Event.EventId = 1 + BinaryEventIdOffset;
		Event.EventData = SerializeBinaryAugmentaScene(AugmentaScene);
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = true;

		ClusterManager->EmitClusterEventBinary(Event, true);
	}
	else {
		FDisplayClusterClusterEventJson Event;

		Event.Name = "SceneUpdated";
		Event.Type = "-1";
		Event.Category = "Augmenta";
		Event.Parameters = SerializeJsonAugmentaScene(AugmentaScene);
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = true;

		ClusterManager->EmitClusterEventJson(Event, true);
	}
}

void ALiveLinkAugmentaClusterManager::SendObjectEnteredClusterEvent(const FLiveLinkAugmentaObject AugmentaObject)
{
	if (bUseBinaryClusterEvents)
	{
		FDisplayClusterClusterEventBinary Event;

		Event.EventId = 2 + BinaryEventIdOffset;
		Event.EventData = SerializeBinaryAugmentaObject(AugmentaObject);
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = false;

		ClusterManager->EmitClusterEventBinary(Event, true);
	}
	else {
		FDisplayClusterClusterEventJson Event;

		Event.Name = "ObjectEntered";
		Event.Type = FString::FromInt(AugmentaObject.Id);
		Event.Category = "Augmenta";
		Event.Parameters = SerializeJsonAugmentaObject(AugmentaObject);
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = true;

		ClusterManager->EmitClusterEventJson(Event, true);
	}
}

void ALiveLinkAugmentaClusterManager::SendObjectUpdatedClusterEvent(const FLiveLinkAugmentaObject AugmentaObject)
{
	if(bUseBinaryClusterEvents)
	{
		FDisplayClusterClusterEventBinary Event;

		Event.EventId = 3 + BinaryEventIdOffset;
		Event.EventData = SerializeBinaryAugmentaObject(AugmentaObject);
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = false;

		ClusterManager->EmitClusterEventBinary(Event, true);
	}
	else {
		FDisplayClusterClusterEventJson Event;

		Event.Name = "ObjectUpdated";
		Event.Type = FString::FromInt(AugmentaObject.Id);
		Event.Category = "Augmenta";
		Event.Parameters = SerializeJsonAugmentaObject(AugmentaObject);
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = true;

		ClusterManager->EmitClusterEventJson(Event, true);
	}
}

void ALiveLinkAugmentaClusterManager::SendObjectLeftClusterEvent(const int32 ObjectId)
{
	if(bUseBinaryClusterEvents)
	{
		FDisplayClusterClusterEventBinary Event;

		Event.EventId = 4 + BinaryEventIdOffset;
		Event.EventData = SerializeBinaryObjectId(ObjectId);
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = false;

		ClusterManager->EmitClusterEventBinary(Event, true);
	}
	else {
		FDisplayClusterClusterEventJson Event;

		Event.Name = "ObjectLeft";
		Event.Type = FString::FromInt(ObjectId);
		Event.Category = "Augmenta";
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = true;

		ClusterManager->EmitClusterEventJson(Event, true);
	}
}

void ALiveLinkAugmentaClusterManager::SendSourceDestroyedClusterEvent()
{
	if (bUseBinaryClusterEvents)
	{
		FDisplayClusterClusterEventBinary Event;

		Event.EventId = BinaryEventIdOffset;
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = true;

		ClusterManager->EmitClusterEventBinary(Event, true);
	}
	else {
		FDisplayClusterClusterEventJson Event;

		Event.Name = "SourceDestroyed";
		Event.Type = "";
		Event.Category = "Augmenta";
		Event.bIsSystemEvent = false;
		Event.bShouldDiscardOnRepeat = true;

		ClusterManager->EmitClusterEventJson(Event, true);
	}
}

void ALiveLinkAugmentaClusterManager::OnClusterEventJson_Implementation(const FDisplayClusterClusterEventJson& Event)
{
	if (Event.Name == "SceneUpdated")
	{
		OnAugmentaSceneUpdated.Broadcast(DeserializeJsonAugmentaScene(Event.Parameters));
	}
	else if (Event.Name == "ObjectEntered")
	{
		OnAugmentaObjectEntered.Broadcast(DeserializeJsonAugmentaObject(Event.Parameters));
	}
	else if (Event.Name == "ObjectUpdated")
	{
		OnAugmentaObjectUpdated.Broadcast(DeserializeJsonAugmentaObject(Event.Parameters));
	}
	else if (Event.Name == "ObjectLeft")
	{
		OnAugmentaObjectLeft.Broadcast(FCString::Atoi(*Event.Type));
	}
	else if (Event.Name == "SourceDestroyed")
	{
		OnAugmentaSourceDestroyed.Broadcast();
	}
}

void ALiveLinkAugmentaClusterManager::OnClusterEventBinary_Implementation(
	const FDisplayClusterClusterEventBinary& Event)
{
	if (Event.EventId == BinaryEventIdOffset)
	{
		OnAugmentaSourceDestroyed.Broadcast();
	}
	else if(Event.EventId == BinaryEventIdOffset + 1)
	{
		OnAugmentaSceneUpdated.Broadcast(DeserializeBinaryAugmentaScene(Event.EventData));
	}
	else if (Event.EventId == BinaryEventIdOffset + 2)
	{
		OnAugmentaObjectEntered.Broadcast(DeserializeBinaryAugmentaObject(Event.EventData));
	}
	else if (Event.EventId == BinaryEventIdOffset + 3)
	{
		OnAugmentaObjectUpdated.Broadcast(DeserializeBinaryAugmentaObject(Event.EventData));
	}
	else if (Event.EventId == BinaryEventIdOffset + 4)
	{
		OnAugmentaObjectLeft.Broadcast(DeserializeBinaryObjectId(Event.EventData));
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////
// Json Serialization
//////////////////////////////////////////////////////////////////////////////////////////////

TMap<FString, FString> ALiveLinkAugmentaClusterManager::SerializeJsonAugmentaScene(const FLiveLinkAugmentaScene AugmentaScene)
{
	TMap<FString, FString> EventData;

	EventData.Add("PosX", FString::SanitizeFloat(AugmentaScene.Position.X));
	EventData.Add("PosY", FString::SanitizeFloat(AugmentaScene.Position.Y));
	EventData.Add("PosZ", FString::SanitizeFloat(AugmentaScene.Position.Z));

	EventData.Add("SizeX", FString::SanitizeFloat(AugmentaScene.Size.X));
	EventData.Add("SizeY", FString::SanitizeFloat(AugmentaScene.Size.Y));
	EventData.Add("SizeZ", FString::SanitizeFloat(AugmentaScene.Size.Z));

	EventData.Add("VideoPosX", FString::SanitizeFloat(AugmentaScene.VideoPosition.X));
	EventData.Add("VideoPosY", FString::SanitizeFloat(AugmentaScene.VideoPosition.Y));
	EventData.Add("VideoPosZ", FString::SanitizeFloat(AugmentaScene.VideoPosition.Z));

	EventData.Add("VideoSizeX", FString::SanitizeFloat(AugmentaScene.VideoSize.X));
	EventData.Add("VideoSizeY", FString::SanitizeFloat(AugmentaScene.VideoSize.Y));
	EventData.Add("VideoSizeZ", FString::SanitizeFloat(AugmentaScene.VideoSize.Z));

	EventData.Add("VideoResX", FString::FromInt(AugmentaScene.VideoResolution.X));
	EventData.Add("VideoResY", FString::FromInt(AugmentaScene.VideoResolution.Y));

	return EventData;
}

FLiveLinkAugmentaScene ALiveLinkAugmentaClusterManager::DeserializeJsonAugmentaScene(const TMap<FString, FString> EventData)
{
	FLiveLinkAugmentaScene AugmentaScene;

	FString TmpString = *EventData.Find("PosX");	AugmentaScene.Position.X = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("PosY");	AugmentaScene.Position.Y = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("PosZ");	AugmentaScene.Position.Z = FCString::Atod(*TmpString);

	TmpString = *EventData.Find("SizeX");	AugmentaScene.Size.X = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("SizeY");	AugmentaScene.Size.Y = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("SizeZ");	AugmentaScene.Size.Z = FCString::Atod(*TmpString);

	TmpString = *EventData.Find("VideoPosX");	AugmentaScene.VideoPosition.X = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("VideoPosY");	AugmentaScene.VideoPosition.Y = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("VideoPosZ");	AugmentaScene.VideoPosition.Z = FCString::Atod(*TmpString);

	TmpString = *EventData.Find("VideoSizeX"); AugmentaScene.VideoSize.X = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("VideoSizeY"); AugmentaScene.VideoSize.Y = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("VideoSizeZ"); AugmentaScene.VideoSize.Z = FCString::Atod(*TmpString);

	TmpString = *EventData.Find("VideoResX"); AugmentaScene.VideoResolution.X = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("VideoResY"); AugmentaScene.VideoResolution.Y = FCString::Atod(*TmpString);

	return AugmentaScene;
}

TMap<FString, FString> ALiveLinkAugmentaClusterManager::SerializeJsonAugmentaObject(
	const FLiveLinkAugmentaObject AugmentaObject)
{
	TMap<FString, FString> EventData;

	EventData.Add("Id", FString::FromInt(AugmentaObject.Id));
	EventData.Add("Uid", FString::FromInt(AugmentaObject.Uid));
	//EventData.Add("Age", FString::SanitizeFloat(AugmentaObject.Age));
	EventData.Add("Confidence", FString::SanitizeFloat(AugmentaObject.Confidence));
	EventData.Add("Height", FString::SanitizeFloat(AugmentaObject.Height));
	EventData.Add("CentroidPosX", FString::SanitizeFloat(AugmentaObject.CentroidPosition.X));
	EventData.Add("CentroidPosY", FString::SanitizeFloat(AugmentaObject.CentroidPosition.Y));
	EventData.Add("CentroidPosZ", FString::SanitizeFloat(AugmentaObject.CentroidPosition.Z));
	EventData.Add("PosX", FString::SanitizeFloat(AugmentaObject.Position.X));
	EventData.Add("PosY", FString::SanitizeFloat(AugmentaObject.Position.Y));
	EventData.Add("PosZ", FString::SanitizeFloat(AugmentaObject.Position.Z));
	EventData.Add("RotationX", FString::SanitizeFloat(AugmentaObject.Rotation.X));
	EventData.Add("RotationY", FString::SanitizeFloat(AugmentaObject.Rotation.Y));
	EventData.Add("RotationZ", FString::SanitizeFloat(AugmentaObject.Rotation.Z));
	EventData.Add("RotationW", FString::SanitizeFloat(AugmentaObject.Rotation.W));
	EventData.Add("SizeX", FString::SanitizeFloat(AugmentaObject.Size.X));
	EventData.Add("SizeY", FString::SanitizeFloat(AugmentaObject.Size.Y));
	EventData.Add("SizeZ", FString::SanitizeFloat(AugmentaObject.Size.Z));

	return EventData;
}

FLiveLinkAugmentaObject ALiveLinkAugmentaClusterManager::DeserializeJsonAugmentaObject(
	const TMap<FString, FString> EventData)
{
	FLiveLinkAugmentaObject AugmentaObject;

	FString TmpString = *EventData.Find("Id");	 AugmentaObject.Id = FCString::Atoi(*TmpString);
	TmpString = *EventData.Find("Uid");	 AugmentaObject.Uid = FCString::Atoi(*TmpString);
	//TmpString = *EventData.Find("Age"); AugmentaObject.Age = FCString::Atof(*TmpString);
	TmpString = *EventData.Find("Confidence");		AugmentaObject.Confidence = FCString::Atof(*TmpString);
	TmpString = *EventData.Find("Height");		AugmentaObject.Height = FCString::Atof(*TmpString);
	TmpString = *EventData.Find("CentroidPosX");		AugmentaObject.CentroidPosition.X = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("CentroidPosY");		AugmentaObject.CentroidPosition.Y = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("CentroidPosZ");		AugmentaObject.CentroidPosition.Z = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("PosX");		AugmentaObject.Position.X = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("PosY");		AugmentaObject.Position.Y = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("PosZ");		AugmentaObject.Position.Z = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("RotationX");		AugmentaObject.Rotation.X = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("RotationY");		AugmentaObject.Rotation.Y = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("RotationZ");		AugmentaObject.Rotation.Z = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("RotationW");		AugmentaObject.Rotation.W = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("SizeX");			AugmentaObject.Size.X = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("SizeY");			AugmentaObject.Size.Y = FCString::Atod(*TmpString);
	TmpString = *EventData.Find("SizeZ");			AugmentaObject.Size.Z = FCString::Atod(*TmpString);

	return AugmentaObject;
}


//////////////////////////////////////////////////////////////////////////////////////////////
// Binary Serialization
//////////////////////////////////////////////////////////////////////////////////////////////

TArray<uint8> ALiveLinkAugmentaClusterManager::SerializeBinaryAugmentaScene(const FLiveLinkAugmentaScene AugmentaScene)
{
	TArray<uint8> EventData;

	// Allocate buffer memory
	const uint32 BufferSize = sizeof(AugmentaScene);
	EventData.SetNumUninitialized(BufferSize);

	FMemory::Memcpy(EventData.GetData(), &AugmentaScene, BufferSize);

	return EventData;
}

FLiveLinkAugmentaScene ALiveLinkAugmentaClusterManager::DeserializeBinaryAugmentaScene(const TArray<uint8> EventData)
{
	FLiveLinkAugmentaScene AugmentaScene;

	FMemory::Memcpy(&AugmentaScene, EventData.GetData(), sizeof(AugmentaScene));

	return AugmentaScene;
}

TArray<uint8> ALiveLinkAugmentaClusterManager::SerializeBinaryAugmentaObject(
	const FLiveLinkAugmentaObject AugmentaObject)
{
	TArray<uint8> EventData;

	// Allocate buffer memory
	const uint32 BufferSize = sizeof(AugmentaObject);
	EventData.SetNumUninitialized(BufferSize);

	FMemory::Memcpy(EventData.GetData(), &AugmentaObject, BufferSize);

	return EventData;
}

FLiveLinkAugmentaObject ALiveLinkAugmentaClusterManager::DeserializeBinaryAugmentaObject(const TArray<uint8> EventData)
{
	FLiveLinkAugmentaObject AugmentaObject;

	FMemory::Memcpy(&AugmentaObject, EventData.GetData(), sizeof(AugmentaObject));

	return AugmentaObject;
}

TArray<uint8> ALiveLinkAugmentaClusterManager::SerializeBinaryObjectId(const int32 ObjectId)
{
	TArray<uint8> EventData;

	// Allocate buffer memory
	const uint32 BufferSize = sizeof(int32);
	EventData.SetNumUninitialized(BufferSize);

	FMemory::Memcpy(EventData.GetData(), &ObjectId, BufferSize);

	return EventData;
}

int32 ALiveLinkAugmentaClusterManager::DeserializeBinaryObjectId(const TArray<uint8> EventData)
{
	int32 ObjectId;

	FMemory::Memcpy(&ObjectId, EventData.GetData(), sizeof(int32));

	return ObjectId;
}



