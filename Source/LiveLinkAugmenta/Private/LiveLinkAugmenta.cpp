// Copyright Augmenta, All Rights Reserved.

#include "LiveLinkAugmenta.h"

#define LOCTEXT_NAMESPACE "FLiveLinkAugmentaModule"

DEFINE_LOG_CATEGORY(LogLiveLinkAugmenta);

void FLiveLinkAugmentaModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FLiveLinkAugmentaModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FLiveLinkAugmentaModule, LiveLinkAugmenta)