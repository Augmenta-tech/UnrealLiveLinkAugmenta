// Fill out your copyright notice in the Description page of Project Settings.

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

	//Live link preset to load
	UPROPERTY(EditAnywhere, Category = "Augmenta|Live Link")
	ULiveLinkPreset *LiveLinkPreset;

	//Live link scene name
	UPROPERTY(EditAnywhere, Category = "Augmenta|Live Link")
	FString SceneName;

	///** A delegate that is fired when an Augmenta scene message is received. */
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaSceneUpdatedEvent AugmentaSceneUpdated;

	///** A delegate that is fired when an Augmenta video output (fusion) message is received. */
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaVideoOutputUpdatedEvent AugmentaVideoOutputUpdated;

	///** A delegate that is fired when an Augmenta object entered message is received. */
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent AugmentaObjectEntered;

	///** A delegate that is fired when an Augmenta object updated message is received. */
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent AugmentaObjectUpdated;

	///** A delegate that is fired when an Augmenta object will leave message is received. */
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent AugmentaObjectWillLeave;

	//Augmenta scene data
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Augmenta|Data")
	FLiveLinkAugmentaScene AugmentaScene;

	// Augmenta objects data
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Augmenta|Data")
	TMap<int, FLiveLinkAugmentaObject> AugmentaObjects;

	//Augmenta video output data
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category = "Augmenta|Data")
	FLiveLinkAugmentaVideoOutput AugmentaVideoOutput;

private:

	
	FLiveLinkAugmentaSource* LiveLinkAugmentaSource;

	//FTimerHandle FindSourceTimerHandle;

	void FindLiveLinkSource();

	//Events from Live Link Source
	void OnAugmentaSceneUpdated(FLiveLinkAugmentaScene AugmentaScene);
	void OnAugmentaVideoOutputUpdated(FLiveLinkAugmentaVideoOutput AugmentaVideoOutput);
	void OnAugmentaObjectEntered(FLiveLinkAugmentaObject AugmentaObject);
	void OnAugmentaObjectUpdated(FLiveLinkAugmentaObject AugmentaObject);
	void OnAugmentaObjectWillLeave(FLiveLinkAugmentaObject AugmentaObject);
	void OnLiveLinkSourceClosed();

	//Events to Blueprint
	void SendReceivedEvents();

	bool bIsSceneUpdated = false;
	bool bIsVideoOutputUpdated = false;

	enum EventType {Entered, Updated, WillLeave};
	TMap<int, EventType> AugmentaObjectsReceivedEvents;
	TArray<int> AugmentaObjectsReceivedEventsKeys;

	void AddAugmentaObjectReceivedEvent(int id, EventType type);

	FCriticalSection Mutex;
};
