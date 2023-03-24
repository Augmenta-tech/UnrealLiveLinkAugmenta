// Copyright Augmenta 2023, All Rights Reserved.

#pragma once

#include "LiveLinkAugmentaData.h"
#include "Cluster/IDisplayClusterClusterEventListener.h"

#include "CoreMinimal.h"
#include "LiveLinkAugmentaEventDispatcher.h"
#include "Cluster/IDisplayClusterClusterManager.h"
#include "GameFramework/Actor.h"
#include "LiveLinkAugmentaClusterManager.generated.h"

/** Forward Declarations */
class IDisplayClusterClusterManager;

UCLASS(BlueprintType, Category = "Augmenta")
class LIVELINKAUGMENTA_API ALiveLinkAugmentaClusterManager : public ALiveLinkAugmentaEventDispatcher, public IDisplayClusterClusterEventListener
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALiveLinkAugmentaClusterManager();

	//Augmenta Event Dispatcher to attach to.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Augmenta")
	ALiveLinkAugmentaEventDispatcher* AugmentaEventDispatcher;

	//Whether to use binary or json cluster events.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Augmenta|Cluster Events")
	bool bUseBinaryClusterEvents;

	//Offset binary cluster events id by this value. Useful to avoid overlapping with other binary events.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Augmenta|Cluster Events")
	int BinaryEventIdOffset;

	//Send only the transform, id and age data of the Augmenta objects to improve performance. Only works with json cluster events.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Augmenta|Cluster Events")
	bool bSendReducedObjectData;

protected:

	bool bInitialized;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Augmenta|Cluster")
	void SendSceneUpdatedClusterEvent(const FLiveLinkAugmentaScene AugmentaScene);

	UFUNCTION(BlueprintCallable, Category = "Augmenta|Cluster")
	void SendVideoOutputUpdatedClusterEvent(const FLiveLinkAugmentaVideoOutput AugmentaVideoOutput);

	UFUNCTION(BlueprintCallable, Category = "Augmenta|Cluster")
	void SendObjectEnteredClusterEvent(const FLiveLinkAugmentaObject AugmentaObject);

	UFUNCTION(BlueprintCallable, Category = "Augmenta|Cluster")
	void SendObjectUpdatedClusterEvent(const FLiveLinkAugmentaObject AugmentaObject);

	UFUNCTION(BlueprintCallable, Category = "Augmenta|Cluster")
	void SendObjectLeftClusterEvent(const FLiveLinkAugmentaObject AugmentaObject);

	UFUNCTION(BlueprintCallable, Category = "Augmenta|Cluster")
	void SendSourceDestroyedClusterEvent();

	TMap<FString, FString> SerializeJsonAugmentaScene(const FLiveLinkAugmentaScene AugmentaScene);
	FLiveLinkAugmentaScene DeserializeJsonAugmentaScene(const TMap<FString, FString> EventData);

	TMap<FString, FString> SerializeJsonAugmentaVideoOutput(const FLiveLinkAugmentaVideoOutput AugmentaVideoOutput);
	FLiveLinkAugmentaVideoOutput DeserializeJsonAugmentaVideoOutput(const TMap<FString, FString> EventData);

	TMap<FString, FString> SerializeJsonAugmentaObject(const FLiveLinkAugmentaObject AugmentaObject);
	FLiveLinkAugmentaObject DeserializeJsonAugmentaObject(const TMap<FString, FString> EventData);

	TArray<uint8> SerializeBinaryAugmentaScene(const FLiveLinkAugmentaScene AugmentaScene);
	FLiveLinkAugmentaScene DeserializeBinaryAugmentaScene(const TArray<uint8> EventData);

	TArray<uint8> SerializeBinaryAugmentaVideoOutput(const FLiveLinkAugmentaVideoOutput AugmentaVideoOutput);
	FLiveLinkAugmentaVideoOutput DeserializeBinaryAugmentaVideoOutput(const TArray<uint8> EventData);

	TArray<uint8> SerializeBinaryAugmentaObject(const FLiveLinkAugmentaObject AugmentaObject);
	FLiveLinkAugmentaObject DeserializeBinaryAugmentaObject(const TArray<uint8> EventData);

	IDisplayClusterClusterManager* ClusterManager;

public:

	UFUNCTION(BlueprintNativeEvent)
	void OnClusterEventJson(const FDisplayClusterClusterEventJson& Event);

	UFUNCTION(BlueprintNativeEvent)
	void OnClusterEventBinary(const FDisplayClusterClusterEventBinary& Event);

};
