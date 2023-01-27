// Copyright Augmenta, All Rights Reserved.

#pragma once

#include "LiveLinkAugmenta.h"
#include "LiveLinkAugmentaData.h"

#include "Containers/CircularQueue.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiveLinkAugmentaManager.generated.h"

#define AUGMENTAEVENTQUEUECAPACITY 1024

/** Forward Declarations */
class ULiveLinkPreset;
class FLiveLinkAugmentaSource;

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentaSceneUpdatedEvent, const FLiveLinkAugmentaScene, AugmentaScene);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentaObjectUpdatedEvent, const FLiveLinkAugmentaObject, AugmentaObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentaVideoOutputUpdatedEvent, const FLiveLinkAugmentaVideoOutput, AugmentaVideoOutput);

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
	float SourceSearchDelay;

	// Is this manager connected to a Live Link Source
	UPROPERTY(VisibleAnywhere, Category = "Augmenta|Live Link")
	bool bIsConnected;

	// A delegate that is fired when an Augmenta scene message is received.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaSceneUpdatedEvent OnAugmentaSceneUpdated;

	// A delegate that is fired when an Augmenta video output (fusion) message is received.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaVideoOutputUpdatedEvent OnAugmentaVideoOutputUpdated;

	// A delegate that is fired when a new Augmenta object entered the scene.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent OnAugmentaObjectEntered;

	// A delegate that is fired when an Augmenta object has been updated.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent OnAugmentaObjectUpdated;

	// A delegate that is fired when an Augmenta object has left the scene.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent OnAugmentaObjectLeft;

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
