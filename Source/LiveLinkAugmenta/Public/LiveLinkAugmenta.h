// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogLiveLinkAugmenta, Log, All);

class FLiveLinkAugmentaModule : public IModuleInterface
{
public:

	static FLiveLinkAugmentaModule& Get()
	{
		return FModuleManager::Get().LoadModuleChecked<FLiveLinkAugmentaModule>(TEXT("LiveLinkAugmenta"));
	}

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
