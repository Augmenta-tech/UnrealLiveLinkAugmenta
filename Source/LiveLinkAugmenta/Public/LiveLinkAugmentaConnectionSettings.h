// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkAugmentaConnectionSettings.generated.h"

USTRUCT()
struct LIVELINKAUGMENTA_API FLiveLinkAugmentaConnectionSettings
{
	GENERATED_BODY()

	/** IP address of the Augmenta server */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	FString IPAddress = TEXT("127.0.0.1");

	/** UDP port number */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	uint16 UDPPortNumber = 12000;

	/** Maximum rate (in Hz) at which to ask the Augmenta server to update */
	UPROPERTY(EditAnywhere, Category = "Connection Settings", meta = (ClampMin = 1, ClampMax = 1000))
	uint32 LocalUpdateRateInHz = 120;

	/** Augmenta scene name */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	FString SceneName = TEXT("AugmentaMain");

	/** Time before a not updated object is removed */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	float TimeoutDuration = 1;
};
