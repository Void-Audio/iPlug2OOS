# VST3 to AUv3 pita
Alright this shit sucks kinda bad but here we go.

We have to build an entire extra app, which is just a wrapper / compatibility layer. Additionally, even upon successful build of said AUv3 wrapper app, Logic Pro may still reject it if it is not signed properly.

- copy `SMTG_AddVST3AuV3.cmake` and `SMTG_CodeSign.cmake` cmake modules from `vst3sdk` into a `cmake/modules` folder in the root directory of `vst3plugin`
	- probably need to slightly alter the `SMTG_AddVST3AuV3.cmake`, at least in my experience I had to add `target_include_directories(${app-target} PRIVATE \path\to\vst3sdk)` where `${app-target}` includes the wrapper files (like `ViewController.m/h`, or `AppDelegate.m/h`)
- create `audiounitconfig.h` file in the root directory of `vst3plugin`
	- "`AUWRAPPER_CHANGE`: change all corresponding entries according to your plugin"
- modify `CMakeLists.txt` or add somewhere the commands to build the AUv3 wrapper library

Good idea to acquaint oneself with [Apple's AudioToolbox Framework](https://developer.apple.com/documentation/audiotoolbox) since many things needed for this conversion to work properly are documented there. An example: `struct` describing the unique and identifying IDs of an audio component

```objective-c
/*
 * componentType -- A 4-char code identifying the generic type of
 *                  an audio component.
 * componentSubType -- A 4-char code identifying the a specific
 *                     individual component.
 *                     type/subtype/manufacturer triples must be
 *                     globally unique.
 * componentManufacturer -- Vendor identification.
 * componentFlags -- Must be set to zero unless a known specific
 *                   value is requested.
 * componentFlagsMask -- Must be set to zero unless a known
 *                       specific value is requested.
*/
typedef struct AudioComponentDescription {
    OSType    componentType; // A 4-char code identifying the generic type of an audio component.
    OSType    componentSubType; // A 4-char code identifying the a specific individual component. type/subtype/manufacturer triples must be globally unique.
    OSType    componentManufacturer; // Vendor identification.
    UInt32    componentFlags; // Must be set to zero unless a known specific value is requested.
    UInt32    componentFlagsMask; // Must be set to zero unless a known specific value is requested.
} AudioComponentDescription;
```

this struct is filled in `viewDidLoad()` by the values specified in `audiounitconfig.s` . Checkout the [CoreAudioKit Framework](https://developer.apple.com/documentation/coreaudiokit)  in which [`AUViewController`](https://developer.apple.com/documentation/coreaudiokit/auviewcontroller/) may be found as well as the [AUAudioUnitViewConfiguration](https://developer.apple.com/documentation/coreaudiokit/auaudiounitviewconfiguration) protocol. The following excerpt is found on the `AUViewController` page:
>### [Subclassing Notes](https://developer.apple.com/documentation/coreaudiokit/auviewcontroller/#Subclassing-Notes)
>If an audio unit provides a custom view controller, the UI Audio Unit extension must implement a subclass of `AUViewController` and implement the [`AUAudioUnitFactory`](https://developer.apple.com/documentation/AudioToolbox/AUAudioUnitFactory) protocol inside the subclass.

who knows, maybe
```objective-c
/*-----------------------------------*\
	AUv3WrapperViewController          
\*-----------------------------------*/
@interface AUv3WrapperViewController ()
{
	IPtr<IPlugView> plugView;
	IPtr<AUPlugFrame> plugFrame;
	IPtr<IEditController> editController;
	BOOL isAttached;
}
@end
```
