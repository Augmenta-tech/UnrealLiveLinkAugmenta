// Copyright Augmenta 2023, All Rights Reserved.

#pragma once

#include "LiveLinkAugmenta.h"
#include "LiveLinkAugmentaData.h"

#include "Containers/CircularQueue.h"

#include "CoreMinimal.h"
#include "LiveLinkAugmentaEventDispatcher.h"
#include "GameFramework/Actor.h"
#include "LiveLinkAugmentaManager.generated.h"

#define AUGMENTAEVENTQUEUECAPACITY 2048

/** Forward Declarations */
class ULiveLinkPreset;
class FLiveLinkAugmentaSource;

// Structure used to store Augmenta event data in the circular queue to transfer them between threads
USTRUCT(BlueprintType) //BlueprintType to get access in BP
struct FAugmentaEventData
{
	GENERATED_USTRUCT_BODY()

	//Those are not UPROPERTY() in the circular queue 
	//so do not store UE Actor or UE Object pointers in this struct!

	// Id of the Augmenta object this event refers to. -1 = AugmentaScene, -2 = AugmentaVideoOutput
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Augmenta|Event Data")
	int ObjectId = 0;

	// Type of event : 0 = SceneUpdate, 1 = VideoOutputUpdate, 2 = ObjectEnter, 3 = ObjectUpdate, 4 = ObjectLeave
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Augmenta|Event Data")
	int EventType = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Augmenta|Event Data")
	FLiveLinkAugmentaObject AugmentaObject;
};

// Augmenta event data queue used to transfer events between threads
UCLASS()
class UAugmentaEventDataQueue
	: public UObject
{
	GENERATED_BODY()

public:

	//This is not UPROPERTY() so do not store UE Actor or UE Object pointers in this struct!
	TCircularQueue<FAugmentaEventData> Events;

	UAugmentaEventDataQueue(const FObjectInitializer& ObjectInitializer)
		: Super(ObjectInitializer)
		, Events(AUGMENTAEVENTQUEUECAPACITY)
	{ }

	UAugmentaEventDataQueue(FVTableHelper& Helper)
		: Super(Helper)
		, Events(AUGMENTAEVENTQUEUECAPACITY)
	{ }

	//1024 = max number of items queue can store, can store more as you dequeue all items of course
};

UCLASS(BlueprintType, Category = "Augmenta")
class LIVELINKAUGMENTA_API ALiveLinkAugmentaManager : public ALiveLinkAugmentaEventDispatcher
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

	// Live link preset to load at start. Can be left empty to avoid replacing default Live Link preset.
	UPROPERTY(EditAnywhere, Category = "Augmenta|Live Link")
	ULiveLinkPreset *LiveLinkPreset;

	// Scene name of the Live link Augmenta source to attach to.
	UPROPERTY(EditAnywhere, Category = "Augmenta|Live Link")
	FName SceneName;

	// Time in seconds between each Live Link source search.
	UPROPERTY(EditAnywhere, Category = "Augmenta|Live Link")
	float SourceSearchDelay;

	// Is this manager currently connected to a Live Link Source ?
	UPROPERTY(VisibleAnywhere, Category = "Augmenta|Live Link")
	bool bIsConnected;


	// The event count threshold (in percentage of the event queue capacity) above which warnings are issued
	UPROPERTY(EditAnywhere, Category = "Augmenta|Events", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float EventQueueCapacityWarningThreshold = 0.8f;

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

	/**
	*  Get the Augmenta Object with specific Id
	*  @param  AugmentaObject       The returned AugmentaObject
	*  @param  Id					The desired object Id
	*  @return FALSE if no object with the desired Id was found
	*/
	UFUNCTION(BlueprintPure, Category = "Augmenta|Objects")
	bool GetAugmentaObjectById(FLiveLinkAugmentaObject& AugmentaObject, int Id);

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

	UPROPERTY()
	UAugmentaEventDataQueue* AugmentaEventDataQueue = nullptr;

private:

	FLiveLinkAugmentaSource* LiveLinkAugmentaSource;

	FTimerHandle SearchSourceTimerHandle;

	void SearchLiveLinkSource();

	void OnLiveLinkAugmentaSceneUpdated(FLiveLinkAugmentaScene AugmentaScene);
	void OnLiveLinkAugmentaVideoOutputUpdated(FLiveLinkAugmentaVideoOutput AugmentaVideoOutput);
	void OnLiveLinkAugmentaObjectEntered(FLiveLinkAugmentaObject AugmentaObject);
	void OnLiveLinkAugmentaObjectUpdated(FLiveLinkAugmentaObject AugmentaObject);
	void OnLiveLinkAugmentaObjectWillLeave(FLiveLinkAugmentaObject AugmentaObject);
	void OnLiveLinkAugmentaSourceDestroyed();

	void PropagateLiveLinkEvents();
	void PropagateLiveLinkEventFromEventData(FAugmentaEventData EventData);

	UPROPERTY()
	TArray<FAugmentaEventData> EventDataCache;
};
