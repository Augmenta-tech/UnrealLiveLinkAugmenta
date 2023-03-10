// Copyright Augmenta 2023, All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkAugmentaConnectionSettings.generated.h"

USTRUCT()
struct LIVELINKAUGMENTA_API FLiveLinkAugmentaConnectionSettings
{
	GENERATED_BODY()

	/** IP address of the receiving UDP socket */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	FString IPAddress = TEXT("0.0.0.0");

	/** Port number of the receiving UDP socket */
	UPROPERTY(EditAnywhere, Category = "Connection Settings")
	uint16 PortNumber = 12000;

	/** Local update rate of the source */
	UPROPERTY(EditAnywhere, Category = "Connection Settings", meta = (ClampMin = 1, ClampMax = 1000))
	uint32 LocalUpdateRateInHz = 120;

	/** Augmenta scene name */
	UPROPERTY(EditAnywhere, Category = "Augmenta Settings")
	FName SceneName = TEXT("AugmentaMain");
};
