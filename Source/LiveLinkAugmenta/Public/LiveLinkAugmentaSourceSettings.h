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

	/** Augmenta scene name. */
	UPROPERTY(VisibleAnywhere, Category = "Augmenta")
	FName SceneName;

	/** Use bounding box size as object scale. */
	UPROPERTY(EditAnywhere, Category = "Augmenta|Augmenta Objects")
	bool bApplyObjectSize = true;

	/** Offset object position vertically according to the bounding box height.  */
	UPROPERTY(EditAnywhere, Category = "Augmenta|Augmenta Objects")
	bool bApplyObjectHeight = false;

	/** Automatically remove Augmenta objects if Augmenta messages are no longer received.  */
	UPROPERTY(EditAnywhere, Category = "Augmenta|Augmenta Objects")
	bool bAutoRemoveObjects = true;

	/** Disable the creation and update of Live Link subjects from received Augmenta data.
	 ** This can improve performances when using the Augmenta Manager instead of the Live Link subjects. */
	UPROPERTY(EditAnywhere, Category = "Augmenta|Optimization")
	bool bDisableSubjectsUpdate = false;

	/** Live Link source reference */
	FLiveLinkAugmentaSource* SourceReference;
};
