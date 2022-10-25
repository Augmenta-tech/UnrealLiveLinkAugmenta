// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LiveLinkSourceSettings.h"
#include "LiveLinkAugmentaSourceSettings.generated.h"

class FLiveLinkAugmentaSource;

UCLASS()
class LIVELINKAUGMENTA_API ULiveLinkAugmentaSourceSettings : public ULiveLinkSourceSettings
{
	GENERATED_BODY()

public:

	/** Augmenta scene name */
	UPROPERTY(VisibleAnywhere, Category = "Augmenta")
	FString SceneName;

	/** Live Link source reference */
	FLiveLinkAugmentaSource* SourceReference;
};
