// Copyright Augmenta 2023, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkAugmentaData.generated.h"

// Augmenta data as per official spec :
// https://github.com/Augmenta-tech/Augmenta/wiki/Data

USTRUCT(BlueprintType, Category = "Augmenta|Data")
struct LIVELINKAUGMENTA_API FLiveLinkAugmentaObject
{
	GENERATED_BODY()

	/** The slot Id of the object (smallest available id).	*/
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	int32 Id = -1;

	/** The unique Id of the object (ex: 42nd object to enter is assigned uid=41). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	int32 Uid = -1;

	/** The alive time in seconds. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float Age = -1;

	/** The presence value of the object.
	 0 means the point just arrived or left, 1 means the point is fully in the scene. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float Presence = 0;

	/** The absolute position of the centroid of the object. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector CentroidPosition = FVector::ZeroVector;

	/** The absolute position of the object. It is the bounding box position if the bounding box is used, the centroid position otherwise. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector Position = FVector::ZeroVector;

	/** The absolute rotation of the bounding box of the object. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FQuat Rotation = FQuat::Identity;

	/** The size of the bounding box of the object. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector Size = FVector::OneVector;
};


/**
 * A structure to hold data for the Augmenta Scene.
 */
USTRUCT(BlueprintType, Category = "Augmenta|Data")
struct LIVELINKAUGMENTA_API FLiveLinkAugmentaScene
{
	GENERATED_BODY()

	/** The maximum number of objects (or slots) in the scene. 0 = unlimited. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene")
	float MaxNumObj = 0;

	/** The scene absolute position. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene")
	FVector Position = FVector::ZeroVector;

	/** The scene rotation along Unreal axis. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene")
	FQuat Rotation = FQuat::Identity;

	/** The scene size along Unreal axis. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene")
	FVector Size = FVector::ZeroVector;

	/** The video output resolution in pixels. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene|Video")
	FIntPoint VideoResolution = FIntPoint::ZeroValue;
};
