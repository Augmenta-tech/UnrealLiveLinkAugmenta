# [Augmenta](https://www.augmenta-tech.com) [Unreal](https://www.unrealengine.com) [Live Link](https://docs.unrealengine.com/5.0/en-US/live-link-in-unreal-engine/) Plugin

This repo contains the Unreal Live Link Augmenta Plugin only. It is currently compiled with Unreal version **5.4**.

An example project using this plugin can be found [here](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo).

## Installation

1. Close your Unreal project.

2. Download this project :
    1. Git User : Add this repository as a submodule in a `Plugins` folder at the root of your Unreal project (where your Content folder is). The path to this repository should look like `YourProjectName\Plugins\UnrealLiveLinkAugmenta\`.

    2. Non Git User : Download zip and unzip this project in a `Plugins` folder at the root of your Unreal project (where your Content folder is). The path to this project should look like `YourProjectName\Plugins\UnrealLiveLinkAugmenta\`.

3. Launch your Unreal project. It should ask you to build UnrealLiveLinkAugmenta, click yes and wait for the build to finish and the Unreal editor to open.

## Implemented Features

### OSC Protocol V2
[V2 Wiki](https://github.com/Augmenta-tech/Augmenta/wiki/Data)

The Augmenta data is stored in the LiveLinkAugmentaSource :
- The AugmentaScene contains the scene data.
- The AugmentaVideoOutput contains the video output data.
- The AugmentaObjects contains the augmenta objects data, stored in a TMap with the object id as key and AugmentaObject as value. 

### LiveLinkAugmentaSource

The Live Link Augmenta Source has the following parameters :

| Parameter | Description |
| --- | --- |
| TimeoutDuration | An existing Augmenta object which has not been updated for this duration will be removed. |
| ApplyObjectScale | If true, the Augmenta object bounding box size is used as the scale of the subject transform. |
| ApplyObjectHeight | If true, the position of the subject is moved up by half the Augmenta object height (i.e. it is moved up so that the bottom of the bounding box is at the Augmenta scene level). |
| OffsetObjectPositionOnCentroid | If true, the centroid of the Augmenta object is used for the subject transform position. Otherwise the bounding box center is used. This parameter is only used when ApplyObjectScale is true. If ApplyObjectScale is false, the subject transform position will always be the centroid of the Augmenta object. |
| DisableSubjectsUpdate | Disable the creation and update of Live Link subjects from received Augmenta data to improve performance when they are not needed. |

## Usage

The Augmenta data received by this plugin can be used via the Live Link subjects, or the AugmentaManager. Both usages are described below. In both case you need to create the Live Link Source.

### Creating the Live Link Augmenta source

1. Open the Live Link window by clicking Window -> Virtual Production -> Live Link.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo/blob/marketplace-demo/Resources/Documentation/Images/LiveLinkSourceCreation_1.jpg)

2. In the Live Link window click on Source, LiveLinkAugmenta Source, enter your IP address, port and scene name, then click Add.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo/blob/marketplace-demo/Resources/Documentation/Images/LiveLinkSourceCreation_2.jpg)

3. If you are receiving Augmenta data (from a node, Fusion, or the simulator), you should see the received Augmenta elements in the subjects list.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo/blob/marketplace-demo/Resources/Documentation/Images/LiveLinkSourceCreation_3.jpg)


### Using the Live Link subjects

This is the standard Live Link usage. The Augmenta elements (scene, videoOutput and objects) are exposed as Live Link subjects with the Transform role.

You can use those subjects to control the transforms of actors in your scene through the LiveLinkComponentController.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo/blob/marketplace-demo/Resources/Documentation/Images/LiveLinkComponentController.jpg)

You can check the LiveLinkAugmentaDemo_LiveLinkSubjects level of the [Unreal Live Link Augmenta Demo project](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo) for an example of this usage.


### Using the Augmenta Manager

The Augmenta Manager exposes the complete Augmenta data of the Live Link Source to C++ or Blueprints for advanced usage such as spawning objects at runtime or using more advanced Augmenta data. Additionally, the Augmenta Manager transfer Augmenta events from the Live Link source thread to the game thread (for blueprints).

The parameters of the Augmenta manager are described below.

| Parameter | Description |
| --- | --- |
| LiveLinkPreset | Live link preset to load at start. This can be left empty to avoid replacing an existing loaded Live Link preset. |
| SceneName | Scene name of the Live link Augmenta source to attach to. |
| SourceSearchDelay | Delay between each source search, as long as the manager is not connected to a Live link source. |
| IsConnected | Read only, true if the manager is connected to a Live link source. |
| EventQueueCapacityWarningThreshold | The event count threshold (in percentage of the event queue capacity) above which warnings are issued. If the event queue capacity is reached, it means the Live link source receives messages faster than the manager can propagate them. This can happens if your Augmenta data rate is too high, or your game framerate is too low. In this case, incoming events from the Live link source might be missed by the manager. The event queue currently has a capacity of 2047. This means you should not receive more than 2047 augmenta messages during every frame of your game. This can be manually increased in the LiveLinkAugmentaManager.h file. |

The exposed methods of the Augmenta manager are described below.

| Method | Description |
| --- | --- |
| GetAugmentaScene | Returns a copy of the Augmenta scene. |
| GetAugmentaVideoOutput | Returns a copy of the Augmenta video output. |
| GetAugmentaObjects | Returns a copy of the Augmenta objects map. |
| GetAugmentaObjectById | Returns a copy of an Augmenta object with a specific Id (if it exists). |
| GetAugmentaObjectsCount | Returns the current number of Augmenta objects. |

The exposed events of the Augmenta manager are described below.

| Event | Description |
| --- | --- |
| OnAugmentaSceneUpdated | Fired when the Augmenta scene is updated. |
| OnAugmentaVideoOutputUpdated | Fired when the Augmenta video output is updated. |
| OnAugmentaObjectEntered | Fired when a new Augmenta object entered the scene. |
| OnAugmentaObjectUpdated | Fired when an Augmenta object as been updated. |
| OnAugmentaObjectLeft | Fired when an Augmenta object left the scene. |
| OnAugmentaSourceDestroyed | Fired when the Augmenta source that this manager is connected to is destroyed. |

An example of usage of the Augmenta manager is shown in the LiveLinkAugmentaDemo_AugmentaManager level of the [Unreal Live Link Augmenta Demo project](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo).

In this level, a blueprint derived from the Augmenta manager is added in the scene to load a Live Link preset and a custom AugmentaVisualizer blueprint connects to the manager events in order to display debug objects for the Augmenta scene, video outputs and objects.

>Note that if you are only using the Augmenta manager and not the Live link subjects, you can disable the Live link subjects update in the Augmenta source.

### Using the Augmenta Cluster Manager

The AugmentaClusterManager is a C++ class that binds to an existing AugmentaManager to send the event from the AugmentaManager through a nDisplay cluster via cluster events. This allows to propagate the Augmenta events through the cluster in a synchronized manner. This framework is described in the diagram below.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo/blob/marketplace-demo/Resources/Documentation/Images/AugmentaClusterManagerDiagram.jpg)

An example of usage of the Augmenta cluster manager is shown in the LiveLinkAugmentaDemo_AugmentaClusterManager_nDisplay level of the [Unreal Live Link Augmenta Demo project](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo).

In this level, the AugmentaClusterManager propagate cluster events from the incoming Augmenta events from the AugmentaManager, while the AugmentaClusterVisualizer listen to the AugmentaClusterManager events for the instantiation and update of the visualization objects.

The Live Link Augmenta Cluster Manager has the following parameters :

| Parameter | Description |
| --- | --- |
| AugmentaManager | The Augmenta manager to link this cluster manager to. |
| UseBinaryClusterEvents | If true, Augmenta events are propagated to the cluster using binary cluster events, otherwise json cluster events are used. Binary cluster events are better for data throughput and latency. |
| BinaryEventIdOffset | Binary cluster events are identified by a unique id (an integer). Different binary cluster events sharing the same id will lead to interpretation issues and potential crashes. The cluster manager use ids 0 to 5 by default, but they can be offsetted to prevent overlapping with other event ids using this parameter. |
| SendReducedObjectData | If true, only the transform, id and age data of the Augmenta objects will be sent through cluster events to improve performance. This only works when using json cluster events. |

### Creating and loading Live Link presets

Live Link presets can be created in the Live Link window by clicking on Presets and Save as. 

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo/blob/marketplace-demo/Resources/Documentation/Images/LiveLinkPresetCreation.jpg))

This allows to save your Live Link sources configuration and reload it at runtime. To do so, you can specify a default preset for your project to load at start up in the Project Settings -> Live Link.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo/blob/marketplace-demo/Resources/Documentation/Images/LiveLinkProjectSettings.jpg))

Or if you are using an Augmenta manager, you can specify the Live Link preset to load in the Augmenta manager directly. Note that if you use several Augmenta managers in the same level, you should have only one of them responsible for loading a Live Link preset. 


