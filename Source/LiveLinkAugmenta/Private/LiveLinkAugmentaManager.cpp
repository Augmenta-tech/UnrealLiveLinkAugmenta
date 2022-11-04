// Fill out your copyright notice in the Description page of Project Settings.

#include "LiveLinkAugmentaManager.h"

#include "LiveLinkClient.h"
#include "LiveLinkPreset.h"

#include "LiveLinkAugmentaSource.h"

// Sets default values
ALiveLinkAugmentaManager::ALiveLinkAugmentaManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneName = "AugmentaMain";
}

// Called when the game starts or when spawned
void ALiveLinkAugmentaManager::BeginPlay()
{
	Super::BeginPlay();
	
	//Apply Live Link preset
	if (IsValid(LiveLinkPreset)) {
		LiveLinkPreset->ApplyToClientLatent();
	}

	FindLiveLinkSource();
}

// Called every frame
void ALiveLinkAugmentaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (LiveLinkAugmentaSource == nullptr) {
		FindLiveLinkSource();
	}
	else {
		SendReceivedEvents();
	}
}

void ALiveLinkAugmentaManager::FindLiveLinkSource()
{

	if (IModularFeatures::Get().IsModularFeatureAvailable(ILiveLinkClient::ModularFeatureName))
	{
		FLiveLinkClient& LiveLinkClient = IModularFeatures::Get().GetModularFeature<FLiveLinkClient>(ILiveLinkClient::ModularFeatureName);

		TArray<FGuid> AllSources;
		AllSources.Append(LiveLinkClient.GetSources());
		//AllSources.Append(LiveLinkClient.GetVirtualSources());

		for (auto& Source : AllSources) {

			//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Found source %s of type %s"), *Source.ToString(), *LiveLinkClient.GetSourceType(Source).ToString());
			
			if (LiveLinkClient.GetSourceType(Source).ToString() == "Augmenta") {

				ULiveLinkAugmentaSourceSettings* AugmentaSourceSettings = Cast<ULiveLinkAugmentaSourceSettings>(LiveLinkClient.GetSourceSettings(Source));

				if (AugmentaSourceSettings->SceneName == SceneName) {
					LiveLinkAugmentaSource = AugmentaSourceSettings->SourceReference;
					break;
				}
			}
		}
	}

	if (LiveLinkAugmentaSource == nullptr) {

		//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Could not find an Augmenta source named %s."), *SceneName);
	}
	else {

		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Found Augmenta source named %s."), *SceneName);

		//Bind events
		LiveLinkAugmentaSource->OnLiveLinkAugmentaSceneUpdated.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaSceneUpdated);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaVideoOutputUpdated.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaVideoOutputUpdated);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaObjectEntered.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaObjectEntered);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaObjectUpdated.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaObjectUpdated);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaObjectWillLeave.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaObjectWillLeave);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaSourceClosed.BindUObject(this, &ALiveLinkAugmentaManager::OnLiveLinkSourceClosed);
	}
}

void ALiveLinkAugmentaManager::OnAugmentaSceneUpdated(FLiveLinkAugmentaScene NewAugmentaScene)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Scene Updated."));

	AugmentaScene = NewAugmentaScene;

	bIsSceneUpdated = true;
}

void ALiveLinkAugmentaManager::OnAugmentaVideoOutputUpdated(FLiveLinkAugmentaVideoOutput NewAugmentaVideoOutput)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: VideoOutput Updated."));

	AugmentaVideoOutput = NewAugmentaVideoOutput;

	bIsVideoOutputUpdated = true;
}

void ALiveLinkAugmentaManager::OnAugmentaObjectEntered(FLiveLinkAugmentaObject AugmentaObject)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Object Entered."));

	if (AugmentaObjects.Contains(AugmentaObject.Id)) {
		AugmentaObjects[AugmentaObject.Id] = AugmentaObject;
	}
	else {
		AugmentaObjects.Emplace(AugmentaObject.Id, AugmentaObject);
	}

	AddAugmentaObjectReceivedEvent(AugmentaObject.Id, EventType::Entered);
}

void ALiveLinkAugmentaManager::OnAugmentaObjectUpdated(FLiveLinkAugmentaObject AugmentaObject)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Object Updated."));

	if (AugmentaObjects.Contains(AugmentaObject.Id)) {
		AugmentaObjects[AugmentaObject.Id] = AugmentaObject;
	}
	else {
		AugmentaObjects.Emplace(AugmentaObject.Id, AugmentaObject);
	}

	AddAugmentaObjectReceivedEvent(AugmentaObject.Id, EventType::Updated);
}

void ALiveLinkAugmentaManager::OnAugmentaObjectWillLeave(FLiveLinkAugmentaObject AugmentaObject)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Object Will Leave."));

	if (AugmentaObjects.Contains(AugmentaObject.Id)) {
		AugmentaObjects.Remove(AugmentaObject.Id);
	}

	AddAugmentaObjectReceivedEvent(AugmentaObject.Id, EventType::WillLeave);
}

void ALiveLinkAugmentaManager::OnLiveLinkSourceClosed()
{
	LiveLinkAugmentaSource = nullptr;

	//Mark all objects for removal
	for (auto& o : AugmentaObjects) {
		AddAugmentaObjectReceivedEvent(o.Key, EventType::WillLeave);
	}
}

void ALiveLinkAugmentaManager::SendReceivedEvents()
{
	/* Scene */
	if (bIsSceneUpdated) {
		if (AugmentaSceneUpdated.IsBound()) {
			AugmentaSceneUpdated.Broadcast(AugmentaScene);
		}

		bIsSceneUpdated = false;
	}

	/* Video output */
	if (bIsVideoOutputUpdated) {
		if (AugmentaVideoOutputUpdated.IsBound()) {
			AugmentaVideoOutputUpdated.Broadcast(AugmentaVideoOutput);
		}

		bIsVideoOutputUpdated = false;
	}

	/* Objects */

	//Lock the received event list
	FScopeLock lock(&Mutex);

	//Get the list of present ids
	AugmentaObjectsReceivedEvents.GenerateKeyArray(AugmentaObjectsReceivedEventsKeys);

	//Dispatch corresponding events
	for (auto& key : AugmentaObjectsReceivedEventsKeys)
	{
		switch (AugmentaObjectsReceivedEvents[key]) {
		case EventType::Entered:
			if (AugmentaObjectEntered.IsBound()) {
				AugmentaObjectEntered.Broadcast(AugmentaObjects[key]);
			}
			break;

		case EventType::Updated:
			if (AugmentaObjectUpdated.IsBound()) {
				AugmentaObjectUpdated.Broadcast(AugmentaObjects[key]);
			}
			break;

		case EventType::WillLeave:
			if (AugmentaObjectWillLeave.IsBound()) {
				AugmentaObjectWillLeave.Broadcast(AugmentaObjects[key]);
			}
			break;

		}
	}

	//Empty the received events list
	AugmentaObjectsReceivedEvents.Empty();
}

void ALiveLinkAugmentaManager::AddAugmentaObjectReceivedEvent(int id, EventType type) 
{
	if (AugmentaObjectsReceivedEvents.Contains(id)) {
		AugmentaObjectsReceivedEvents.Remove(id);
	}

	AugmentaObjectsReceivedEvents.Emplace(id, type);
}

