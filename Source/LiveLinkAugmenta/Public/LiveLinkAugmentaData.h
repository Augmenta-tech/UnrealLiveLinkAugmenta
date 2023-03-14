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

	/** The scene frame number. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	int32 Frame = 0;

	/** The unique Id for each object. (ex: 42nd object to enter is assigned pid=41). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	int32 Id = 0;

	/**
	 * The ordered Id for each object.
	 * (ex: if 3 objects are on stage, 43rd object still present has oid=2).
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	int32 Oid = 0;

	/** The alive time in seconds. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float Age = 0;

	/** The position projected to the ground (normalized). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector2D Centroid = FVector2D::ZeroVector;

	/** The speed and direction vector (normalized). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector2D Velocity = FVector2D::ZeroVector;

	/** The object rotation w.r.t the horizontal axis (right). Range is 0 to 360. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float Orientation = 0;

	/** The center coordinate of the bounding box (normalized). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector2D BoundingRectPos = FVector2D::ZeroVector;

	/** The size of the bounding box (normalized). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector2D BoundingRectSize = FVector2D::ZeroVector;

	/** The rotation of the bounding box w.r.t the horizontal axis (right). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float BoundingRectRotation = 0;;

	/** The height of the Object (in m) (absolute). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float Height = 0;

	/** The highest point placement (normalized). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FVector2D Highest = FVector2D::ZeroVector;

	/** The distance of the object to the sensor (in m) (absolute). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float Distance = 0;

	/** The reflection value from a Lidar sensor. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	float Reflectivity = 0;

	/** The time at which the object was last updated. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object")
	FDateTime LastUpdateTime = FDateTime::Now();

	/** The absolute position of the object. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object|Transform")
	FVector Position = FVector::ZeroVector;

	/** The absolute rotation of the object. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object|Transform")
	FQuat Rotation = FQuat::Identity;

	/** The local scale of the object. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Object|Transform")
	FVector Scale = FVector::ZeroVector;
};


/**
 * A structure to hold data for the Augmenta Scene.
 */
USTRUCT(BlueprintType, Category = "Augmenta|Data")
struct LIVELINKAUGMENTA_API FLiveLinkAugmentaScene
{
	GENERATED_BODY()

	/** The Time in frame number. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene")
	int32 Frame = 0;

	/** The number of objects in the scene. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene")
	int32 ObjectCount = 0;

	/** The scene size in meters. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene")
	FVector2D Size = FVector2D::ZeroVector;

	/** The scene absolute position. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene|Transform")
	FVector Position = FVector::ZeroVector;

	/** The scene rotation. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene|Transform")
	FQuat Rotation = FQuat::Identity;

	/** The scene scale along Unreal axis. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|Scene|Transform")
	FVector Scale = FVector::ZeroVector;
};

/**
 * A structure to hold data for the Augmenta VideoOutput (Fusion).
 */
USTRUCT(BlueprintType, Category = "Augmenta|Data")
struct LIVELINKAUGMENTA_API FLiveLinkAugmentaVideoOutput
{
	GENERATED_BODY()

	/** The offset from the scene top left (in m). */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|VideoOutput")
	FVector2D Offset = FVector2D::ZeroVector;

	/** The size (in m). X is Width and Y is Height. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|VideoOutput")
	FVector2D Size = FVector2D::ZeroVector;

	/** The resolution in pixels. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|VideoOutput")
	FIntPoint Resolution = FIntPoint::ZeroValue;

	/** The video output absolute position. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|VideoOutput|Transform")
	FVector Position = FVector::ZeroVector;

	/** The video output rotation. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|VideoOutput|Transform")
	FQuat Rotation = FQuat::Identity;

	/** The video output scale along Unreal axis. */
	UPROPERTY(BlueprintReadWrite, Category = "Augmenta|VideoOutput|Transform")
	FVector Scale = FVector::ZeroVector;
};
