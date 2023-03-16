// Copyright Augmenta 2023, All Rights Reserved.

#include "LiveLinkAugmentaManager.h"

#include "LiveLinkClient.h"
#include "LiveLinkPreset.h"

#include "LiveLinkAugmentaSource.h"

#include "Engine/World.h"
#include "TimerManager.h"
#include "Features/IModularFeatures.h"


// Sets default values
ALiveLinkAugmentaManager::ALiveLinkAugmentaManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneName = "AugmentaMain";
	bIsConnected = false;
	SourceSearchDelay = 2.0f;
}

// Called when the game starts or when spawned
void ALiveLinkAugmentaManager::BeginPlay()
{
	Super::BeginPlay();

	//Create Event data queue
	AugmentaEventDataQueue = NewObject<UAugmentaEventDataQueue>(this, TEXT("Augmenta Event Data Queue"));

	//Apply Live Link preset
	if (IsValid(LiveLinkPreset)) {
		LiveLinkPreset->ApplyToClientLatent();
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Applying Live Link preset..."));
	} else
	{
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Live Link preset is empty. Make sure to apply the correct Live Link preset yourself (using the Default preset in Project Settings -> Live Link for example)."));
	}

	//Wait 5 seconds for the preset to load then try to find Live Link Source
	GetWorld()->GetTimerManager().SetTimer(SearchSourceTimerHandle, this, &ALiveLinkAugmentaManager::SearchLiveLinkSource, SourceSearchDelay, false);
}

// Called every frame
void ALiveLinkAugmentaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PropagateLiveLinkEvents();
}

bool ALiveLinkAugmentaManager::GetAugmentaScene(FLiveLinkAugmentaScene& AugmentaScene)
{
	if(bIsConnected)
	{
		AugmentaScene = LiveLinkAugmentaSource->GetAugmentaScene();
		return true;
	}

	return false;
}

bool ALiveLinkAugmentaManager::GetAugmentaObjects(TMap<int, FLiveLinkAugmentaObject>& AugmentaObjects)
{
	if (bIsConnected)
	{
		AugmentaObjects = LiveLinkAugmentaSource->GetAugmentaObjects();
		return true;
	}

	return false;
}

bool ALiveLinkAugmentaManager::GetAugmentaObjectById(FLiveLinkAugmentaObject& AugmentaObject, int Id)
{
	if (bIsConnected)
	{
		return LiveLinkAugmentaSource->GetAugmentaObjectById(AugmentaObject, Id);
	}

	return false;
}

int ALiveLinkAugmentaManager::GetAugmentaObjectsCount()
{
	return bIsConnected ? LiveLinkAugmentaSource->GetAugmentaObjectsCount() : 0;
}

bool ALiveLinkAugmentaManager::GetAugmentaVideoOutput(FLiveLinkAugmentaVideoOutput& AugmentaVideoOutput)
{
	if (bIsConnected)
	{
		AugmentaVideoOutput = LiveLinkAugmentaSource->GetAugmentaVideoOutput();
		return true;
	}

	return false;
}

void ALiveLinkAugmentaManager::SearchLiveLinkSource()
{

	if (IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		FLiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(ILiveLinkClient::ModularFeatureName);

		TArray<FGuid> AllSources;
		AllSources.Append(LiveLinkClient.GetSources());
		//AllSources.Append(LiveLinkClient.GetVirtualSources());

		for (auto& Source : AllSources) {

			UE_LOG(LogLiveLinkAugmenta, Verbose, TEXT("LiveLinkAugmentaManager: Found source %s of type %s"), *Source.ToString(), *LiveLinkClient.GetSourceType(Source).ToString());

			if (LiveLinkClient.GetSourceType(Source).ToString() == "Augmenta") {

				ULiveLinkAugmentaSourceSettings* AugmentaSourceSettings = Cast<ULiveLinkAugmentaSourceSettings>(LiveLinkClient.GetSourceSettings(Source));

				if (AugmentaSourceSettings && AugmentaSourceSettings->SceneName == SceneName) {
					LiveLinkAugmentaSource = AugmentaSourceSettings->SourceReference;
					break;
				}
			}
		}
	}

	if (LiveLinkAugmentaSource == nullptr) {
		//Try again in sourceCheckPeriod seconds
		GetWorld()->GetTimerManager().SetTimer(SearchSourceTimerHandle, this, &ALiveLinkAugmentaManager::SearchLiveLinkSource, SourceSearchDelay, false);
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Could not find an Augmenta source named %s. Trying again in %f seconds."), *SceneName.ToString(), SourceSearchDelay);
	}
	else {
		GetWorld()->GetTimerManager().ClearTimer(SearchSourceTimerHandle);
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Found Augmenta source named %s."), *SceneName.ToString());

		//Bind events
		LiveLinkAugmentaSource->OnLiveLinkAugmentaSceneUpdated.BindUObject(this, &ALiveLinkAugmentaManager::OnLiveLinkAugmentaSceneUpdated);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaVideoOutputUpdated.BindUObject(this, &ALiveLinkAugmentaManager::OnLiveLinkAugmentaVideoOutputUpdated);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaObjectEntered.BindUObject(this, &ALiveLinkAugmentaManager::OnLiveLinkAugmentaObjectEntered);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaObjectUpdated.BindUObject(this, &ALiveLinkAugmentaManager::OnLiveLinkAugmentaObjectUpdated);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaObjectWillLeave.BindUObject(this, &ALiveLinkAugmentaManager::OnLiveLinkAugmentaObjectWillLeave);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaSourceDestroyed.BindUObject(this, &ALiveLinkAugmentaManager::OnLiveLinkAugmentaSourceDestroyed);

		bIsConnected = true;
	}
}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaSceneUpdated(FLiveLinkAugmentaScene NewAugmentaScene)
{

	if(ensure(AugmentaEventDataQueue))
	{
		FAugmentaEventData NewEventData;
		NewEventData.EventType = 0;
		NewEventData.ObjectId = -1;

		AugmentaEventDataQueue->Events.Enqueue(NewEventData);
	}

	UE_LOG(LogLiveLinkAugmenta, Verbose, TEXT("LiveLinkAugmentaManager: Received event from Live Link: Scene updated."));
}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaVideoOutputUpdated(FLiveLinkAugmentaVideoOutput NewAugmentaVideoOutput)
{

	if (ensure(AugmentaEventDataQueue))
	{
		FAugmentaEventData NewEventData;
		NewEventData.EventType = 1;
		NewEventData.ObjectId = -2;

		AugmentaEventDataQueue->Events.Enqueue(NewEventData);
	}

	UE_LOG(LogLiveLinkAugmenta, VeryVerbose, TEXT("LiveLinkAugmentaManager: Live Link VideoOutput updated."));
}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaObjectEntered(FLiveLinkAugmentaObject AugmentaObject)
{

	if (ensure(AugmentaEventDataQueue))
	{
		FAugmentaEventData NewEventData;
		NewEventData.EventType = 2;
		NewEventData.ObjectId = AugmentaObject.Id;
		NewEventData.AugmentaObject = AugmentaObject;

		AugmentaEventDataQueue->Events.Enqueue(NewEventData);
	}

	UE_LOG(LogLiveLinkAugmenta, VeryVerbose, TEXT("LiveLinkAugmentaManager: Received event from Live Link: Object %d entered."), AugmentaObject.Id);
}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaObjectUpdated(FLiveLinkAugmentaObject AugmentaObject)
{

	if (ensure(AugmentaEventDataQueue))
	{
		FAugmentaEventData NewEventData;
		NewEventData.EventType = 3;
		NewEventData.ObjectId = AugmentaObject.Id;
		NewEventData.AugmentaObject = AugmentaObject;

		AugmentaEventDataQueue->Events.Enqueue(NewEventData);
	}

	UE_LOG(LogLiveLinkAugmenta, VeryVerbose, TEXT("LiveLinkAugmentaManager: Received event from Live Link: Object %d updated."), AugmentaObject.Id);

}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaObjectWillLeave(FLiveLinkAugmentaObject AugmentaObject)
{

	if (ensure(AugmentaEventDataQueue))
	{
		FAugmentaEventData NewEventData;
		NewEventData.EventType = 4;
		NewEventData.ObjectId = AugmentaObject.Id;
		NewEventData.AugmentaObject = AugmentaObject;

		AugmentaEventDataQueue->Events.Enqueue(NewEventData);
	}

	UE_LOG(LogLiveLinkAugmenta, VeryVerbose, TEXT("LiveLinkAugmentaManager: Received event from Live Link: Object %d will leave."), AugmentaObject.Id);
}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaSourceDestroyed()
{
	bIsConnected = false;
	LiveLinkAugmentaSource = nullptr;

	OnAugmentaSourceDestroyed.Broadcast();

	//Start searching for a new Live Link Source
	GetWorld()->GetTimerManager().SetTimer(SearchSourceTimerHandle, this, &ALiveLinkAugmentaManager::SearchLiveLinkSource, SourceSearchDelay, false);

	UE_LOG(LogLiveLinkAugmenta, Warning, TEXT("LiveLinkAugmentaManager: Connected Live Link source was destroyed, restarting source search."));
}

void ALiveLinkAugmentaManager::PropagateLiveLinkEvents()
{
	//Do not propagate anything when no source is connected
	if(!bIsConnected)
	{
		return;
	}

	if(ensure(AugmentaEventDataQueue))
	{
		int QueueEventCount = AugmentaEventDataQueue->Events.Count();
		int QueueCapacity = AUGMENTAEVENTQUEUECAPACITY - 1;

		if(QueueEventCount >= QueueCapacity * EventQueueCapacityWarningThreshold)
		{
			UE_LOG(LogLiveLinkAugmenta, Warning, TEXT("LiveLinkAugmentaManager: Events count in the Augmenta event queue is reaching critical level: %d events while the total capacity is %d. You might need to decrease your Augmenta send rate."), QueueEventCount, QueueCapacity);
		}

		EventDataCache.Empty();

		while(AugmentaEventDataQueue->Events.Count() > 0)
		{
			FAugmentaEventData NewEventData;

			//Get event data from queue
			AugmentaEventDataQueue->Events.Dequeue(NewEventData);

			//Check if an event with this object id as already been extracted from the queue this frame
			bool bIsIdAlreadyInList = false;
			int IdIndexInList = -1;

			for(int i=0; i<EventDataCache.Num(); i++)
			{
				if(EventDataCache[i].ObjectId == NewEventData.ObjectId)
				{
					bIsIdAlreadyInList = true;
					IdIndexInList = i;
					break;
				}
			}

			if(bIsIdAlreadyInList)
			{
				//Update already extracted event for this id
				EventDataCache[IdIndexInList].EventType = NewEventData.EventType;
				EventDataCache[IdIndexInList].AugmentaObject = NewEventData.AugmentaObject;
			} else
			{
				//Add new event for this id
				EventDataCache.Add(NewEventData);
			}
		}

		for (FAugmentaEventData Event : EventDataCache)
		{
			PropagateLiveLinkEventFromEventData(Event);
		}

		UE_LOG(LogLiveLinkAugmenta, Verbose, TEXT("LiveLinkAugmentaManager: Propagated %d of %d Augmenta events after sorting."), EventDataCache.Num(), QueueEventCount);
	}
}

void ALiveLinkAugmentaManager::PropagateLiveLinkEventFromEventData(FAugmentaEventData EventData)
{
	//Check if we have a valid Live Link source to propagate data from
	if(!bIsConnected)
	{
		return;
	}

	switch (EventData.EventType)
	{
	case 0: //Scene updated
	{
		const FLiveLinkAugmentaScene AugmentaScene = LiveLinkAugmentaSource->GetAugmentaScene();
		OnAugmentaSceneUpdated.Broadcast(AugmentaScene);
		UE_LOG(LogLiveLinkAugmenta, VeryVerbose, TEXT("LiveLinkAugmentaManager: Propagating Scene Updated event."));
	}
		break;

	case 1: //Video Output updated
	{
		const FLiveLinkAugmentaVideoOutput AugmentaVideoOutput = LiveLinkAugmentaSource->GetAugmentaVideoOutput();
		OnAugmentaVideoOutputUpdated.Broadcast(AugmentaVideoOutput);
		UE_LOG(LogLiveLinkAugmenta, VeryVerbose, TEXT("LiveLinkAugmentaManager: Propagating Video Output Updated event."));
	}
		break;

	case 2: //Object entered
	{
		OnAugmentaObjectEntered.Broadcast(EventData.AugmentaObject);
		UE_LOG(LogLiveLinkAugmenta, VeryVerbose, TEXT("LiveLinkAugmentaManager: Propagating Object Entered event for object %d."), EventData.ObjectId);
	}
		break;

	case 3: //Object updated
	{
		OnAugmentaObjectUpdated.Broadcast(EventData.AugmentaObject);
		UE_LOG(LogLiveLinkAugmenta, VeryVerbose, TEXT("LiveLinkAugmentaManager: Propagating Object Updated event for object %d."), EventData.ObjectId);
	}
		break;

	case 4: //Object left
	{
		OnAugmentaObjectLeft.Broadcast(EventData.AugmentaObject);
		UE_LOG(LogLiveLinkAugmenta, VeryVerbose, TEXT("LiveLinkAugmentaManager: Propagating Object Left event for object %d."), EventData.ObjectId);
	}
		break;

	default:

		break;
	}
}

