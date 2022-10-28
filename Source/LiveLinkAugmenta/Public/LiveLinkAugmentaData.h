// Copyright Epic Games, Inc. All Rights Reserved.

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
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		int32 Frame;

	/** The unique Id for each object. (ex: 42nd object to enter is assigned pid=41). */
	UPROPERTY(BlueprintReadOnly, DisplayName = "Id", Category = "Augmenta|Object")
		int32 Id;

	/**
	 * The ordered Id for each object.
	 * (ex: if 3 objects are on stage, 43rd object still present has oid=2).
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		int32 Oid;

	/** The alive time in seconds. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		float Age;

	/** The position projected to the ground (normalized). */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		FVector2D Centroid;

	/** The speed and direction vector (normalized). */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		FVector2D Velocity;

	/** The object rotation w.r.t the horizontal axis (right). Range is 0 to 360. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		float Orientation;

	/** The center coordinate of the bounding box (normalized). */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		FVector2D BoundingRectPos;

	/** The size of the bounding box (normalized). */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		FVector2D BoundingRectSize;

	/** The rotation of the bounding box w.r.t the horizontal axis (right). */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		float BoundingRectRotation;

	/** The height of the Object (in m) (absolute). */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		float Height;

	/** The highest point placement (normalized). */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		FVector2D Highest;

	/** The distance of the object to the sensor (in m) (absolute). */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		float Distance;

	/** The reflection value from a Lidar sensor. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		float Reflectivity;

	/** The time at which the object was last updated. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object")
		FDateTime LastUpdateTime;

	/** The absolute position of the object. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object|Transform")
		FVector Position;

	/** The absolute rotation of the object. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object|Transform")
		FQuat Rotation;

	/** The local scale of the object. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Object|Transform")
		FVector Scale;
};


/**
 * A structure to hold data for the Augmenta Scene.
 */
USTRUCT(BlueprintType, Category = "Augmenta|Data")
struct LIVELINKAUGMENTA_API FLiveLinkAugmentaScene
{
	GENERATED_BODY()

	/** The Time in frame number. */
	UPROPERTY(BlueprintReadOnly, DisplayName = "Frame", Category = "Augmenta|Scene")
		int32 Frame;

	/** The number of objects in the scene. */
	UPROPERTY(BlueprintReadOnly, DisplayName = "ObjectCount", Category = "Augmenta|Scene")
		int32 ObjectCount;

	/** The scene size in meters. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Scene")
		FVector2D Size;

	/** The scene absolute position. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Scene|Transform")
		FVector Position;

	/** The scene rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Scene|Transform")
		FQuat Rotation;

	/** The scene scale along Unreal axis. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|Scene|Transform")
		FVector Scale;
};

/**
 * A structure to hold data for the Augmenta VideoOutput (Fusion).
 */
USTRUCT(BlueprintType, Category = "Augmenta|Data")
struct LIVELINKAUGMENTA_API FLiveLinkAugmentaVideoOutput
{
	GENERATED_BODY()

	/** The offset from the scene top left (in m). */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|VideoOutput")
		FVector2D Offset;

	/** The size (in m). X is Width and Y is Height. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|VideoOutput")
		FVector2D Size;

	/** The resolution in pixels. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|VideoOutput")
		FIntPoint Resolution;

	/** The video output absolute position. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|VideoOutput|Transform")
		FVector Position;

	/** The video output rotation. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|VideoOutput|Transform")
		FQuat Rotation;

	/** The video output scale along Unreal axis. */
	UPROPERTY(BlueprintReadOnly, Category = "Augmenta|VideoOutput|Transform")
		FVector Scale;
};
