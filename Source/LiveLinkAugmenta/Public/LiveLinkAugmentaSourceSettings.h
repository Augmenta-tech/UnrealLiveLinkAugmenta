// Copyright Augmenta 2023, All Rights Reserved.

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
	FName SceneName;

	/** Use bounding box size as object scale and box position as object position. */
	UPROPERTY(EditAnywhere, Category = "Augmenta|Augmenta Objects")
	bool bUseBoundingBox = true;

	/** Offset object position vertically according to its height.  */
	UPROPERTY(EditAnywhere, Category = "Augmenta|Augmenta Objects")
	bool bApplyObjectHeight = false;

	/** Disable the creation and update of Live Link subjects from received Augmenta data. */
	UPROPERTY(EditAnywhere, Category = "Augmenta|Optimization")
	bool bDisableSubjectsUpdate = false;

	/** Live Link source reference */
	FLiveLinkAugmentaSource* SourceReference;
};
