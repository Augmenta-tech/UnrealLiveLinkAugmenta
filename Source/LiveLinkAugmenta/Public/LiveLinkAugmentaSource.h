// Copyright Augmenta, All Rights Reserved.

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
	virtual void OnSettingsChanged(ULiveLinkSourceSettings* Settings, const FPropertyChangedEvent& PropertyChangedEvent) override;

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

	// Get Augmenta Scene Name
	FName GetSceneName();

	// Get Augmenta Scene
	FLiveLinkAugmentaScene GetAugmentaScene();

	// Get Augmenta objects map
	TMap<int, FLiveLinkAugmentaObject> GetAugmentaObjects();

	/**
	*  Get the Augmenta Object with specific Id
	*  @param  AugmentaObject       The returned AugmentaObject
	*  @param  Id					The desired object Id
	*  @return FALSE if no object with the desired Id was found
	*/
	bool GetAugmentaObjectById(FLiveLinkAugmentaObject& AugmentaObject, int Id);

	// Get the Augmenta Object count
	int GetAugmentaObjectsCount();

	/**
	*  Get Whether an object with specific Id is present
	*  @param  Id					The desired object Id
	*  @return Whether an object with the desired Id is present
	*/
	bool ContainsId(int Id);

	// Get the Augmenta Video Output
	FLiveLinkAugmentaVideoOutput GetAugmentaVideoOutput();

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

	// Pointer to the settings for this source so we don't have to duplicate data
	ULiveLinkAugmentaSourceSettings* SavedSourceSettings = nullptr;

	// Maximum rate at which to refresh the server
	uint32 LocalUpdateRateInHz = 120;

	// Conversion from meters to unreal units
	const float MetersToUnrealUnits = 100.0f;

	// Maximum inactive time before a point is removed
	float TimeoutDuration;

	// Offset object position vertically according to its height
	bool bApplyObjectHeight;

	// Use bounding box size as object scale
	bool bApplyObjectScale;

	// Use centroid position as position instead of bounding box center when using scale
	bool bOffsetObjectPositionOnCentroid;

	// Augmenta scene parameters
	FName SceneName;
	FLiveLinkAugmentaScene AugmentaScene;

	// Augmenta objects
	TMap<int, FLiveLinkAugmentaObject> AugmentaObjects;

	// Augmenta video output
	FLiveLinkAugmentaVideoOutput AugmentaVideoOutput;

	// OSC Parsing
	void HandleOSCPacket(const OSCPP::Server::Packet& Packet);
	void ReadAugmentaObjectFromOSC(FLiveLinkAugmentaObject* AugmentaObject, OSCPP::Server::ArgStream* Args);
	void UpdateAugmentaObjectExtraFromOSC(OSCPP::Server::ArgStream* Args);
	void AddAugmentaObject(FLiveLinkAugmentaObject AugmentaObject);
	void UpdateAugmentaObject(FLiveLinkAugmentaObject AugmentaObject);
	void RemoveAugmentaObject(FLiveLinkAugmentaObject AugmentaObject);
	void UpdateAugmentaObjectSubject(FLiveLinkAugmentaObject AugmentaObject);
	void RemoveInactiveObjects();

	TArray<int> ObjectsToRemove;

};
