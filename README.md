# [Augmenta](https://www.augmenta-tech.com) [Unreal](https://www.unrealengine.com) [Live Link](https://docs.unrealengine.com/5.0/en-US/live-link-in-unreal-engine/) Plugin

This repo contains the Unreal Live Link Augmenta Plugin only. It is currently compiled with Unreal version **5.1**.

An example project using this plugin can be found [here](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo).

## Installation

1. Close your Unreal project.

2-(Git User). Add this repository as a submodule in a `Plugins` folder at the root of your Unreal project (where your Content folder is). The path to this repository should look like `YourProjectName\Plugins\UnrealLiveLinkAugmenta\`.

2-(Non Git User). Download zip and unzip this project in a `Plugins` folder at the root of your Unreal project (where your Content folder is). The path to this project should look like `YourProjectName\Plugins\UnrealLiveLinkAugmenta\`.

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

## Usage

The Augmenta data received by this plugin can be used via the Live Link subjects, or the AugmentaManager. Both usages are described below. In both case you need to create the Live Link Source.

### Creating the Live Link Augmenta source

1. Open the Live Link window by clicking Window -> Virtual Production -> Live Link.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Documentation/Images/LiveLinkSourceCreation_1.jpg)

2. In the Live Link window click on Source, LiveLinkAugmenta Source, enter your IP address, port and scene name, then click Add.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Documentation/Images/LiveLinkSourceCreation_2.jpg)

3. If you are receiving Augmenta data (from a node, Fusion, or the simulator), you should see the received Augmenta elements in the subjects list.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Documentation/Images/LiveLinkSourceCreation_3.jpg)


### Using the Live Link subjects

This is the standard Live Link usage. The Augmenta elements (scene, videoOutput and objects) are exposed as Live Link subjects with the Transform role.

You can use those subjects to control the transforms of actors in your scene through the LiveLinkComponentController.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Documentation/Images/LiveLinkComponentController.jpg))

You can check the L_LiveLinkAugmentaDemo_LiveLinkSubjects level of the [Unreal Live Link Augmenta Demo project](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo) for an example of this usage.


### Using the Augmenta Manager

The Augmenta Manager exposes the complete Augmenta data of the Live Link Source to C++ or Blueprints for advanced usage such as spawning objects at runtime or using more advanced Augmenta data. Additionally, the Augmenta Manager transfer Augmenta events from the Live Link source thread to the game thread (for blueprints).

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
| OnAugmentaSourceDestroyed | Fired when the Augmenta source that this manager is connected to is being destroyed. |

An example of usage of the Augmenta manager is shown in the L_LiveLinkAugmentaDemo_AugmentaManager level of the [Unreal Live Link Augmenta Demo project](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo).

In this level, a blueprint derived from the Augmenta manager is added in the scene to load a Live Link preset and a custom AugmentaVisualizer blueprint connects to the manager events in order to display debug objects for the Augmenta scene, video outputs and objects.

### Creating and loading Live Link presets

Live Link presets can be created in the Live Link window by clicking on Presets and Save as. 

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Documentation/Images/LiveLinkPresetCreation.jpg))

This allows to save your Live Link sources configuration and reload it at runtime. To do so, you can specify a default preset for your project to load at start up in the Project Settings -> Live Link.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Documentation/Images/LiveLinkProjectSettings.jpg))

Or if you are using an Augmenta manager, you can specify the Live Link preset to load in the Augmenta manager directly. Note that if you use several Augmenta managers in the same level, you should have only one of them responsible for loading a Live Link preset. 


