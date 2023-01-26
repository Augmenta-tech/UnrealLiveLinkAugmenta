// Copyright Augmenta, All Rights Reserved.

#pragma once

#include "LiveLinkSourceFactory.h"
#include "LiveLinkAugmentaSource.h"
#include "LiveLinkAugmentaSourceFactory.generated.h"

UCLASS()
class LIVELINKAUGMENTA_API ULiveLinkAugmentaSourceFactory : public ULiveLinkSourceFactory
{
public:
	GENERATED_BODY()

	virtual FText GetSourceDisplayName() const override;
	virtual FText GetSourceTooltip() const override;

	virtual EMenuType GetMenuType() const override { return EMenuType::SubPanel; }
	virtual TSharedPtr<SWidget> BuildCreationPanel(FOnLiveLinkSourceCreated OnLiveLinkSourceCreated) const override;
	virtual TSharedPtr<ILiveLinkSource> CreateSource(const FString& ConnectionString) const override;

private:
	void CreateSourceFromSettings(FLiveLinkAugmentaConnectionSettings ConnectionSettings, FOnLiveLinkSourceCreated OnSourceCreated) const;
};