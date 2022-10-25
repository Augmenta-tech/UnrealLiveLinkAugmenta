// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "LiveLinkAugmentaConnectionSettings.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/DeclarativeSyntaxSupport.h"

#if WITH_EDITOR
#include "IStructureDetailsView.h"
#endif //WITH_EDITOR

#include "Input/Reply.h"

struct FLiveLinkAugmentaConnectionSettings;

DECLARE_DELEGATE_OneParam(FOnLiveLinkAugmentaConnectionSettingsAccepted, FLiveLinkAugmentaConnectionSettings);

class SLiveLinkAugmentaSourceFactory : public SCompoundWidget
{
	SLATE_BEGIN_ARGS(SLiveLinkAugmentaSourceFactory)
	{}
		SLATE_EVENT(FOnLiveLinkAugmentaConnectionSettingsAccepted, OnConnectionSettingsAccepted)
	SLATE_END_ARGS()

	void Construct(const FArguments& Args);


private:
	FLiveLinkAugmentaConnectionSettings ConnectionSettings;

#if WITH_EDITOR
	TSharedPtr<FStructOnScope> StructOnScope;
	TSharedPtr<IStructureDetailsView> StructureDetailsView;
#endif //WITH_EDITOR

	FReply OnSettingsAccepted();
	FOnLiveLinkAugmentaConnectionSettingsAccepted OnConnectionSettingsAccepted;
};