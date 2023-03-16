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

	/** Time before a not updated object is removed */
	UPROPERTY(EditAnywhere, Category = "Augmenta")
	float TimeoutDuration = 1;

	/** Use bounding box size as object scale */
	UPROPERTY(EditAnywhere, Category = "Augmenta")
	bool bApplyObjectScale = true;

	/** Offset object position vertically according to its height  */
	UPROPERTY(EditAnywhere, Category = "Augmenta")
	bool bApplyObjectHeight = false;

	/** Use centroid position as position instead of bounding box center when using scale */
	UPROPERTY(EditAnywhere, Category = "Augmenta")
	bool bOffsetObjectPositionOnCentroid = true;

	/** Disable the creation and update of Live Link subjects from received Augmenta data */
	UPROPERTY(EditAnywhere, Category = "Augmenta|Optimization")
	bool bDisableSubjectsUpdate = false;

	/** Live Link source reference */
	FLiveLinkAugmentaSource* SourceReference;
};
