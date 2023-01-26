// Copyright Augmenta, All Rights Reserved.

#pragma once

#include "LiveLinkAugmenta.h"
#include "LiveLinkAugmentaData.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiveLinkAugmentaManager.generated.h"

/** Forward Declarations */
class ULiveLinkPreset;
class FLiveLinkAugmentaSource;

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentaSceneUpdatedEvent, const FLiveLinkAugmentaScene, AugmentaScene);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentaObjectUpdatedEvent, const FLiveLinkAugmentaObject, AugmentaObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentaVideoOutputUpdatedEvent, const FLiveLinkAugmentaVideoOutput, AugmentaVideoOutput);

UCLASS(BlueprintType, Category = "Augmenta")
class LIVELINKAUGMENTA_API ALiveLinkAugmentaManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALiveLinkAugmentaManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Live link preset to load
	UPROPERTY(EditAnywhere, Category = "Augmenta|Live Link")
	ULiveLinkPreset *LiveLinkPreset;

	// Live link scene name
	UPROPERTY(EditAnywhere, Category = "Augmenta|Live Link")
	FName SceneName;

	// Time in seconds between each Live Link source search
	UPROPERTY(EditAnywhere, Category = "Augmenta|Live Link")
	float sourceSearchPeriod;

	// Is this manager connected to a Live Link Source
	UPROPERTY(VisibleAnywhere, Category = "Augmenta|Live Link")
	bool bIsConnected;

	// A delegate that is fired when an Augmenta scene message is received.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaSceneUpdatedEvent OnAugmentaSceneUpdated;

	// A delegate that is fired when an Augmenta video output (fusion) message is received.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaVideoOutputUpdatedEvent OnAugmentaVideoOutputUpdated;

	// A delegate that is fired when an Augmenta object entered message is received.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent OnAugmentaObjectEntered;

	// A delegate that is fired when an Augmenta object updated message is received.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent OnAugmentaObjectUpdated;

	// A delegate that is fired when an Augmenta object will leave message is received.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent OnAugmentaObjectWillLeave;

	// Enable Debug Logs
	UPROPERTY(EditAnywhere, Category = "Augmenta|Debug")
	bool bShowDebugLogs;


	/**
	*  Get the Augmenta Scene 
	*  @param  AugmentaScene       Augmenta Scene
	*  @return FALSE if Augmenta Manager source is not valid 
	*/
	UFUNCTION(BlueprintPure, Category = "Augmenta|Scene")
	bool GetAugmentaScene(FLiveLinkAugmentaScene& AugmentaScene);

	/**
	*  Get the Augmenta Objects
	*  @param  AugmentaObjects       Augmenta Objects map. Key is the AugmentaObject Id, Value is the AugmentaObject.
	*  @return FALSE if Augmenta Manager source is not valid
	*/
	UFUNCTION(BlueprintPure, Category = "Augmenta|Objects")
	bool GetAugmentaObjects(TMap<int, FLiveLinkAugmentaObject>& AugmentaObjects);

	// Get Augmenta objects count
	UFUNCTION(BlueprintPure, Category = "Augmenta|Objects")
	int GetAugmentaObjectsCount();

	/**
	*  Get the Augmenta Video Output
	*  @param  AugmentaVideoOutput       Augmenta Video Output
	*  @return FALSE if Augmenta Manager source is not valid
	*/
	UFUNCTION(BlueprintPure, Category = "Augmenta|VideoOutput")
	bool GetAugmentaVideoOutput(FLiveLinkAugmentaVideoOutput& AugmentaVideoOutput);



private:

	FLiveLinkAugmentaSource* LiveLinkAugmentaSource;
	
	FTimerHandle SearchSourceTimerHandle;

	void SearchLiveLinkSource();

	void OnLiveLinkAugmentaSceneUpdated(FLiveLinkAugmentaScene AugmentaScene);
	void OnLiveLinkAugmentaVideoOutputUpdated(FLiveLinkAugmentaVideoOutput AugmentaVideoOutput);
	void OnLiveLinkAugmentaObjectEntered(FLiveLinkAugmentaObject AugmentaObject);
	void OnLiveLinkAugmentaObjectUpdated(FLiveLinkAugmentaObject AugmentaObject);
	void OnLiveLinkAugmentaObjectWillLeave(FLiveLinkAugmentaObject AugmentaObject);
};
