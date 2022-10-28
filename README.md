# [Augmenta](https://www.augmenta-tech.com) [Unreal](https://www.unrealengine.com) [Live Link](https://docs.unrealengine.com/5.0/en-US/live-link-in-unreal-engine/) Plugin

This repo contains the Unreal Live Link Augmenta Plugin only. It is currently compiled with Unreal version **5.0**.

## Installation

### Git User

Add this repository as a submodule in a `*Plugins*` folder at the root of your Unreal project (where your Content folder is). The path to this repository should look like `YourProjectName\Plugins\UnrealLiveLinkAugmenta\`.

### Non Git User

Download zip and unzip this project in a `*Plugins*` folder at the root of your Unreal project (where your Content folder is). The path to this project should look like `YourProjectName\Plugins\UnrealLiveLinkAugmenta\`.

### OSC Protocol V2
[V2 Wiki](https://github.com/Augmenta-tech/Augmenta/wiki/Data)

The Augmenta data is stored in the LiveLinkAugmentaSource :
- The AugmentaScene contains the scene data.
- The AugmentaVideoOutput contains the video output data.
- The AugmentaObjects contains the augmenta objects data, stored in a TMap with the object id as key and AugmentaObject as value. 

## Usage

The Augmenta data received by this plugin can be used via the Live Link subjects, or the AugmentaManager. Both usages are described below. In both case you need to create the Live Link Source.

### Creating the Live Link Augmenta source

1. Open the Live Link window by clicking Window -> Virtual Production -> Live Link.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Images/LiveLinkSourceCreation_1.jpg)

2. In the Live Link window click on Source, LiveLinkAugmenta Source, enter your IP address, port and scene name, then click Add.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Images/LiveLinkSourceCreation_2.jpg)

3. If you are receiving Augmenta data (from a node, Fusion, or the simulator), you should see the received Augmenta elements in the subjects list.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Images/LiveLinkSourceCreation_3.jpg)

### Using the Live Link subjects

This is the standard Live Link usage. The Augmenta elements (scene, videoOutput and objects) are exposed as Live Link subjects with the Transform role.

You can use those subjects to control the transforms of actors in your scene through the LiveLinkComponentController.

![](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta/blob/main/Resources/Images/LiveLinkComponentController.jpg))

You can check the L_LiveLinkAugmentaDemo_LiveLinkSubjects level of the [Unreal Live Link Augmenta Demo project](https://github.com/Augmenta-tech/UnrealLiveLinkAugmenta-Demo) for an example of this usage.


### Using the Augmenta Manager (coming soon)

The Augmenta Manager exposes the complete Augmenta data of the Live Link Source to C++ or Blueprints for advanced usage such as spawning objects at runtime or using more advanced Augmenta data. 

The Augmenta Manager is currently not fully implemented but will be soon.
