
#include <TargetConditionals.h>
#if TARGET_OS_IOS == 1
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

#define IPLUG_AUVIEWCONTROLLER IPlugAUViewController_vBloom
#define IPLUG_AUAUDIOUNIT IPlugAUAudioUnit_vBloom
#import <BloomAU/IPlugAUViewController.h>
#import <BloomAU/IPlugAUAudioUnit.h>

//! Project version number for BloomAU.
FOUNDATION_EXPORT double BloomAUVersionNumber;

//! Project version string for BloomAU.
FOUNDATION_EXPORT const unsigned char BloomAUVersionString[];

@class IPlugAUViewController_vBloom;
