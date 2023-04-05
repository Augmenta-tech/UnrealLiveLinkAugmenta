// Copyright Augmenta 2023, All Rights Reserved.

#pragma once

#include "LiveLinkAugmentaData.h"

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiveLinkAugmentaEventDispatcher.generated.h"

/** Delegates */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentaSceneUpdatedEvent, const FLiveLinkAugmentaScene, AugmentaScene);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentaObjectUpdatedEvent, const FLiveLinkAugmentaObject, AugmentaObject);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAugmentaObjectLeftEvent, const int32, ObjectId);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAugmentaSourceDestroyedEvent);

UCLASS(BlueprintType, Category = "Augmenta")
class LIVELINKAUGMENTA_API ALiveLinkAugmentaEventDispatcher : public AActor
{
	GENERATED_BODY()

public:

	// Sets default values for this actor's properties
	ALiveLinkAugmentaEventDispatcher();

	// A delegate that is fired when an Augmenta scene message is received.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaSceneUpdatedEvent OnAugmentaSceneUpdated;

	// A delegate that is fired when a new Augmenta object entered the scene.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent OnAugmentaObjectEntered;

	// A delegate that is fired when an Augmenta object has been updated.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectUpdatedEvent OnAugmentaObjectUpdated;

	// A delegate that is fired when an Augmenta object has left the scene.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaObjectLeftEvent OnAugmentaObjectLeft;

	// A delegate that is fired when the Augmenta Live Link source is destroyed.
	UPROPERTY(BlueprintAssignable, Category = "Augmenta|Events")
	FAugmentaSourceDestroyedEvent OnAugmentaSourceDestroyed;
};
