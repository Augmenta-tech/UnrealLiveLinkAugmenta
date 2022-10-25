// Fill out your copyright notice in the Description page of Project Settings.

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
}

// Called when the game starts or when spawned
void ALiveLinkAugmentaManager::BeginPlay()
{
	Super::BeginPlay();
	
	//Apply Live Link preset
	if (IsValid(LiveLinkPreset)) {
		LiveLinkPreset->ApplyToClientLatent();
	}

	//Wait 5 seconds for the preset to load then try to find Live Link Source
	GetWorld()->GetTimerManager().SetTimer(FindSourceTimerHandle, this, &ALiveLinkAugmentaManager::FindLiveLinkSource, 5, false);
}

// Called every frame
void ALiveLinkAugmentaManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

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
		//Try again in 2 seconds
		GetWorld()->GetTimerManager().SetTimer(FindSourceTimerHandle, this, &ALiveLinkAugmentaManager::FindLiveLinkSource, 2, false);
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Could not find an Augmenta source named %s. Trying again in 2 seconds."), *SceneName);
	}
	else {
		GetWorld()->GetTimerManager().ClearTimer(FindSourceTimerHandle);
		UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Found Augmenta source named %s."), *SceneName);

		//Bind events
		LiveLinkAugmentaSource->OnLiveLinkAugmentaSceneUpdated.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaSceneUpdated);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaVideoOutputUpdated.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaVideoOutputUpdated);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaObjectEntered.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaObjectEntered);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaObjectUpdated.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaObjectUpdated);
		LiveLinkAugmentaSource->OnLiveLinkAugmentaObjectWillLeave.BindUObject(this, &ALiveLinkAugmentaManager::OnAugmentaObjectWillLeave);
	}
}

void ALiveLinkAugmentaManager::OnAugmentaSceneUpdated(FLiveLinkAugmentaScene NewAugmentaScene)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Scene Updated."));

	AugmentaScene = NewAugmentaScene;
}

void ALiveLinkAugmentaManager::OnAugmentaVideoOutputUpdated(FLiveLinkAugmentaVideoOutput NewAugmentaVideoOutput)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: VideoOutput Updated."));

}

void ALiveLinkAugmentaManager::OnAugmentaObjectEntered(FLiveLinkAugmentaObject AugmentaObject)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Object Entered."));

}

void ALiveLinkAugmentaManager::OnAugmentaObjectUpdated(FLiveLinkAugmentaObject AugmentaObject)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Object Updated."));

}

void ALiveLinkAugmentaManager::OnAugmentaObjectWillLeave(FLiveLinkAugmentaObject AugmentaObject)
{
	//UE_LOG(LogLiveLinkAugmenta, Log, TEXT("LiveLinkAugmentaManager: Object Will Leave."));

}

