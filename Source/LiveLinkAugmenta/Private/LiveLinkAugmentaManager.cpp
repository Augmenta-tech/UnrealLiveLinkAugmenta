// Copyright Augmenta, All Rights Reserved.

#include "LiveLinkAugmentaManager.h"

#include "LiveLinkClient.h"
#include "LiveLinkPreset.h"

#include "LiveLinkAugmentaSource.h"

// Sets default values
ALiveLinkAugmentaManager::ALiveLinkAugmentaManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	SceneName = "AugmentaMain";
	bShowDebugLogs = false;
	bIsConnected = false;
	sourceSearchPeriod = 2.0f;
}

// Called when the game starts or when spawned
void ALiveLinkAugmentaManager::BeginPlay()
{
	Super::BeginPlay();
	
	//Apply Live Link preset
	if (IsValid(LiveLinkPreset)) {
		LiveLinkPreset->ApplyToClientLatent();
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Applying Live Link preset..."));
	} else
	{
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Live Link preset is empty. Make sure to apply the correct Live Link preset yourself (using the Default preset in Project Settings -> Live Link for example)."));
	}

	//Wait 5 seconds for the preset to load then try to find Live Link Source
	GetWorld()->GetTimerManager().SetTimer(SearchSourceTimerHandle, this, &ALiveLinkAugmentaManager::SearchLiveLinkSource, 5, false);
}

// Called every frame
void ALiveLinkAugmentaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
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

			if (bShowDebugLogs) {
				UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Found source %s of type %s"), *Source.ToString(), *LiveLinkClient.GetSourceType(Source).ToString());
			}

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
		GetWorld()->GetTimerManager().SetTimer(SearchSourceTimerHandle, this, &ALiveLinkAugmentaManager::SearchLiveLinkSource, sourceSearchPeriod, false);
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Could not find an Augmenta source named %s. Trying again in %f seconds."), *SceneName.ToString(), sourceSearchPeriod);
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

		bIsConnected = true;
	}
}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaSceneUpdated(FLiveLinkAugmentaScene NewAugmentaScene)
{

	if (bShowDebugLogs)
	{
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Scene updated."));
	}
}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaVideoOutputUpdated(FLiveLinkAugmentaVideoOutput NewAugmentaVideoOutput)
{

	if(bShowDebugLogs)
	{
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: VideoOutput updated."));
	}
}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaObjectEntered(FLiveLinkAugmentaObject AugmentaObject)
{

	if (bShowDebugLogs)
	{
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Object %d entered."), AugmentaObject.Id);
	}
}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaObjectUpdated(FLiveLinkAugmentaObject AugmentaObject)
{

	if (bShowDebugLogs)
	{
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Object %d updated."), AugmentaObject.Id);
	}

}

void ALiveLinkAugmentaManager::OnLiveLinkAugmentaObjectWillLeave(FLiveLinkAugmentaObject AugmentaObject)
{

	if (bShowDebugLogs)
	{
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Object %d will leave."), AugmentaObject.Id);
	}
}

