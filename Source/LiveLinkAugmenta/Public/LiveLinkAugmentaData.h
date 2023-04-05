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

	/** The confidence value of the object. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float Confidence = 0;

	/** The height of the Object (in m) (absolute). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float Height = 0;

	/** The absolute position of the centroid of the object. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector CentroidPosition = FVector::ZeroVector;

	/** The absolute position of the bounding box of the object. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector BoxPosition = FVector::ZeroVector;

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

	/** The scene absolute position. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene")
	FVector Position = FVector::ZeroVector;

	/** The scene size along Unreal axis. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene")
	FVector Size = FVector::ZeroVector;

	/** The video output resolution in pixels. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene|Video")
	FIntPoint VideoResolution = FIntPoint::ZeroValue;

	/** The video output absolute position. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene|Video")
	FVector VideoPosition = FVector::ZeroVector;

	/** The video output size along Unreal axis. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene|Video")
	FVector VideoSize = FVector::ZeroVector;
};
