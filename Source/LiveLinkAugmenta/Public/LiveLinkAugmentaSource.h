// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "ILiveLinkSource.h"
#include "LiveLinkAugmentaConnectionSettings.h"
#include "LiveLinkAugmentaSourceSettings.h"
#include "LiveLinkAugmentaData.h"
#include "Roles/LiveLinkTransformTypes.h"

#include "Delegates/IDelegateInstance.h"
#include "MessageEndpoint.h"
#include "IMessageContext.h"
#include "HAL/ThreadSafeBool.h"
#include "HAL/Runnable.h"

#include "Sockets.h"
#include "SocketSubsystem.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"

#include "oscpp/server.hpp"

struct ULiveLinkAugmentaSettings;

class ILiveLinkClient;

/** Delegates */
DECLARE_DELEGATE_OneParam(FLiveLinkAugmentaSceneUpdatedEvent, FLiveLinkAugmentaScene);
DECLARE_DELEGATE_OneParam(FLiveLinkAugmentaObjectUpdatedEvent, FLiveLinkAugmentaObject);
DECLARE_DELEGATE_OneParam(FLiveLinkAugmentaVideoOutputUpdatedEvent, FLiveLinkAugmentaVideoOutput);

class LIVELINKAUGMENTA_API FLiveLinkAugmentaSource : public ILiveLinkSource, public FRunnable, public TSharedFromThis<FLiveLinkAugmentaSource>
{

public:


	FLiveLinkAugmentaSource(const FLiveLinkAugmentaConnectionSettings& ConnectionSettings);

	virtual ~FLiveLinkAugmentaSource();

	// Begin ILiveLinkSource Interface
	
	virtual void ReceiveClient(ILiveLinkClient* InClient, FGuid InSourceGuid) override;

	virtual void InitializeSettings(ULiveLinkSourceSettings* Settings);

	virtual bool IsSourceStillValid() const override;

	virtual bool RequestSourceShutdown() override;

	virtual FText GetSourceType() const override { return SourceType; };
	virtual FText GetSourceMachineName() const override { return SourceMachineName; }
	virtual FText GetSourceStatus() const override { return SourceStatus; }

	virtual TSubclassOf<ULiveLinkSourceSettings> GetSettingsClass() const override { return ULiveLinkAugmentaSourceSettings::StaticClass(); }

	// End ILiveLinkSource Interface

	// Begin FRunnable Interface

	virtual bool Init() override { return true; }
	virtual uint32 Run() override;
	void Start();
	virtual void Stop() override;
	virtual void Exit() override { }

	// End FRunnable Interface

	/** A delegate that is fired when an Augmenta scene message is generated. */
	FLiveLinkAugmentaSceneUpdatedEvent OnLiveLinkAugmentaSceneUpdated;

	/** A delegate that is fired when an Augmenta Object Entered message is generated. */
	FLiveLinkAugmentaObjectUpdatedEvent OnLiveLinkAugmentaObjectEntered;

	/** A delegate that is fired when an Augmenta Object Updated message is generated. */
	FLiveLinkAugmentaObjectUpdatedEvent OnLiveLinkAugmentaObjectUpdated;

	/** A delegate that is fired when an Augmenta Object Will Leave message is generated. */
	FLiveLinkAugmentaObjectUpdatedEvent OnLiveLinkAugmentaObjectWillLeave;

	/** A delegate that is fired when an Augmenta video output (fusion) message is generated. */
	FLiveLinkAugmentaVideoOutputUpdatedEvent OnLiveLinkAugmentaVideoOutputUpdated;

	FString GetSceneName();

private:

	void Send(FLiveLinkFrameDataStruct* FrameDataToSend, FName SubjectName);

private:
	ILiveLinkClient* Client;

	// Our identifier in LiveLink
	FGuid SourceGuid;

	FMessageAddress ConnectionAddress;

	FText SourceType;
	FText SourceMachineName;
	FText SourceStatus;
	
	// Threadsafe Bool for terminating the main thread loop
	FThreadSafeBool Stopping;
	
	// Thread to run updates on
	FRunnableThread* Thread;
	
	// Name of the updates thread
	FString ThreadName;

	FSocket* Socket;
	ISocketSubsystem* SocketSubsystem;
	FIPv4Endpoint DeviceEndpoint;

	// Size of receive buffer
	const uint32 ReceiveBufferSize = 1024 * 16;

	// Receive buffer for UDP socket
	TArray<uint8> ReceiveBuffer;

	// List of subjects we've already encountered
	TSet<FName> EncounteredSubjects;

	// Deferred start delegate handle
	FDelegateHandle DeferredStartDelegateHandle;

	// Maximum rate at which to refresh the server
	uint32 LocalUpdateRateInHz = 120;

	// Augmenta scene parameters
	FString SceneName;
	FLiveLinkAugmentaScene AugmentaScene;

	// Augmenta objects
	TMap<int, FLiveLinkAugmentaObject> AugmentaObjects;

	// Subjects
	TArray<FName> DesiredSubjects;
	TArray<FName> UndesiredSubjects;

	// OSC Parsing
	void HandleOSCPacket(const OSCPP::Server::Packet& Packet);
};
