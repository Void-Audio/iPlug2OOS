
#include <TargetConditionals.h>
#if TARGET_OS_IOS == 1
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

#define IPLUG_AUVIEWCONTROLLER IPlugAUViewController_vRend
#define IPLUG_AUAUDIOUNIT IPlugAUAudioUnit_vRend
#import <RendAU/IPlugAUViewController.h>
#import <RendAU/IPlugAUAudioUnit.h>

//! Project version number for RendAU.
FOUNDATION_EXPORT double RendAUVersionNumber;

//! Project version string for RendAU.
FOUNDATION_EXPORT const unsigned char RendAUVersionString[];

@class IPlugAUViewController_vRend;
