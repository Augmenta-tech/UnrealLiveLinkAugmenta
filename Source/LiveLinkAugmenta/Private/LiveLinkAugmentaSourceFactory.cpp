// Copyright Augmenta, All Rights Reserved.

#include "LiveLinkAugmentaSourceFactory.h"
#include "LiveLinkAugmentaSource.h"
#include "SLiveLinkAugmentaSourceFactory.h"
#include "LiveLinkAugmentaSourceSettings.h"

#include "ILiveLinkClient.h"
#include "Features/IModularFeatures.h"

#define LOCTEXT_NAMESPACE "LiveLinkAugmentaSourceFactory"

FText ULiveLinkAugmentaSourceFactory::GetSourceDisplayName() const
{
	return LOCTEXT("SourceDisplayName", "LiveLinkAugmenta Source");	
}

FText ULiveLinkAugmentaSourceFactory::GetSourceTooltip() const
{
	return LOCTEXT("SourceTooltip", "Allows creation of multiple LiveLink sources using the Augmenta protocol");
}

TSharedPtr<SWidget> ULiveLinkAugmentaSourceFactory::BuildCreationPanel(FOnLiveLinkSourceCreated InOnLiveLinkSourceCreated) const
{
	return SNew(SLiveLinkAugmentaSourceFactory)
		.OnConnectionSettingsAccepted(FOnLiveLinkAugmentaConnectionSettingsAccepted::CreateUObject(this, &ULiveLinkAugmentaSourceFactory::CreateSourceFromSettings, InOnLiveLinkSourceCreated));
}

TSharedPtr<ILiveLinkSource> ULiveLinkAugmentaSourceFactory::CreateSource(const FString& ConnectionString) const
{
	FLiveLinkAugmentaConnectionSettings ConnectionSettings;
	if (!ConnectionString.IsEmpty())
	{
		FLiveLinkAugmentaConnectionSettings::StaticStruct()->ImportText(*ConnectionString, &ConnectionSettings, nullptr, PPF_None, GLog, TEXT("ULiveLinkAugmentaSourceFactory"));
	}
	return MakeShared<FLiveLinkAugmentaSource>(ConnectionSettings);
}

void ULiveLinkAugmentaSourceFactory::CreateSourceFromSettings(FLiveLinkAugmentaConnectionSettings InConnectionSettings, FOnLiveLinkSourceCreated OnSourceCreated) const
{
	FString ConnectionString;
	FLiveLinkAugmentaConnectionSettings::StaticStruct()->ExportText(ConnectionString, &InConnectionSettings, nullptr, nullptr, PPF_None, nullptr);

	TSharedPtr<FLiveLinkAugmentaSource> SharedPtr = MakeShared<FLiveLinkAugmentaSource>(InConnectionSettings);
	OnSourceCreated.ExecuteIfBound(SharedPtr, MoveTemp(ConnectionString));
}

#undef LOCTEXT_NAMESPACE
