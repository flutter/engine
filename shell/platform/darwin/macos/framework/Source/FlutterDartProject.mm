// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterDartProject.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterDartProject_Internal.h"

#include <vector>

#include "flutter/shell/platform/common/engine_switches.h"

static NSString* const kICUBundlePath = @"icudtl.dat";
static NSString* const kAppBundleIdentifier = @"io.flutter.flutter.app";

// Finds a bundle with the named `bundleID` within `searchURL`.
//
// Returns `nil` if the bundle cannot be found or if errors are encountered.
NSBundle* FLTFrameworkBundleInternal(NSString* bundleID, NSURL* searchURL) {
  NSDirectoryEnumerator<NSURL*>* frameworkEnumerator = [NSFileManager.defaultManager
                 enumeratorAtURL:searchURL
      includingPropertiesForKeys:nil
                         options:NSDirectoryEnumerationSkipsSubdirectoryDescendants |
                                 NSDirectoryEnumerationSkipsHiddenFiles
                    // Skip directories where errors are encountered.
                    errorHandler:nil];

  for (NSURL* candidate in frameworkEnumerator) {
    NSBundle* bundle = [NSBundle bundleWithURL:candidate];
    if ([bundle.bundleIdentifier isEqualToString:bundleID]) {
      return bundle;
    }
  }
  return nil;
}

// Finds a bundle with the named `bundleID`.
//
// `+[NSBundle bundleWithIdentifier:]` is slow, and can take in the order of
// tens of milliseconds in a minimal flutter app, and closer to 100 milliseconds
// in a medium sized Flutter app on an iPhone 13. It is likely that the slowness
// comes from having to traverse and load all bundles known to the process.
// Using `+[NSBundle allframeworks]` and filtering also suffers from the same
// problem.
//
// This implementation is an optimization to first limit the search space to
// `+[NSBundle privateFrameworksURL]` of the main bundle, which is usually where
// frameworks used by this file are placed. If the desired bundle cannot be
// found here, the implementation falls back to
// `+[NSBundle bundleWithIdentifier:]`.
NS_INLINE NSBundle* FLTFrameworkBundleWithIdentifier(NSString* bundleID) {
  NSBundle* bundle = FLTFrameworkBundleInternal(bundleID, NSBundle.mainBundle.privateFrameworksURL);
  if (bundle != nil) {
    return bundle;
  }
  // Fallback to slow implementation.
  return [NSBundle bundleWithIdentifier:bundleID];
}

#pragma mark - Private interface declaration.
@interface FlutterDartProject ()
/**
 Get the Flutter assets name path by pass the bundle. If bundle is nil, we use the main bundle as
 default.
 */
+ (NSString*)flutterAssetsNameWithBundle:(NSBundle*)bundle;
@end

@implementation FlutterDartProject {
  NSBundle* _dartBundle;
  NSString* _assetsPath;
  NSString* _ICUDataPath;
}

- (instancetype)init {
  return [self initWithPrecompiledDartBundle:nil];
}

- (instancetype)initWithPrecompiledDartBundle:(NSBundle*)bundle {
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");

  _dartBundle = bundle ?: FLTFrameworkBundleWithIdentifier(kAppBundleIdentifier);
  if (_dartBundle == nil) {
    // The bundle isn't loaded and can't be found by bundle ID. Find it by path.
    _dartBundle = [NSBundle bundleWithURL:[NSBundle.mainBundle.privateFrameworksURL
                                              URLByAppendingPathComponent:@"App.framework"]];
  }
  if (!_dartBundle.isLoaded) {
    [_dartBundle load];
  }
  _dartEntrypointArguments = [[NSProcessInfo processInfo] arguments];
  // Remove the first element as it's the binary name
  _dartEntrypointArguments = [_dartEntrypointArguments
      subarrayWithRange:NSMakeRange(1, _dartEntrypointArguments.count - 1)];
  return self;
}

- (instancetype)initWithAssetsPath:(NSString*)assets ICUDataPath:(NSString*)icuPath {
  self = [super init];
  NSAssert(self, @"Super init cannot be nil");
  _assetsPath = assets;
  _ICUDataPath = icuPath;
  return self;
}

- (NSString*)assetsPath {
  if (_assetsPath) {
    return _assetsPath;
  }

  // If there's no App.framework, fall back to checking the main bundle for assets.
  NSBundle* assetBundle = _dartBundle ?: [NSBundle mainBundle];
  NSString* flutterAssetsName = [assetBundle objectForInfoDictionaryKey:@"FLTAssetsPath"];
  if (flutterAssetsName == nil) {
    flutterAssetsName = @"flutter_assets";
  }
  NSString* path = [assetBundle pathForResource:flutterAssetsName ofType:@""];
  if (!path) {
    NSLog(@"Failed to find path for \"%@\"", flutterAssetsName);
  }
  return path;
}

- (NSString*)ICUDataPath {
  if (_ICUDataPath) {
    return _ICUDataPath;
  }

  NSString* path = [[NSBundle bundleForClass:[self class]] pathForResource:kICUBundlePath
                                                                    ofType:nil];
  if (!path) {
    NSLog(@"Failed to find path for \"%@\"", kICUBundlePath);
  }
  return path;
}

+ (NSString*)flutterAssetsNameWithBundle:(NSBundle*)bundle {
  if (bundle == nil) {
    bundle = FLTFrameworkBundleWithIdentifier(kAppBundleIdentifier);
  }
  if (bundle == nil) {
    bundle = [NSBundle mainBundle];
  }
  NSString* flutterAssetsName = [bundle objectForInfoDictionaryKey:@"FLTAssetsPath"];
  if (flutterAssetsName == nil) {
    flutterAssetsName = @"Contents/Frameworks/App.framework/Resources/flutter_assets";
  }
  return flutterAssetsName;
}

+ (NSString*)lookupKeyForAsset:(NSString*)asset {
  return [self lookupKeyForAsset:asset fromBundle:nil];
}

+ (NSString*)lookupKeyForAsset:(NSString*)asset fromBundle:(nullable NSBundle*)bundle {
  NSString* flutterAssetsName = [FlutterDartProject flutterAssetsNameWithBundle:bundle];
  return [NSString stringWithFormat:@"%@/%@", flutterAssetsName, asset];
}

+ (NSString*)lookupKeyForAsset:(NSString*)asset fromPackage:(NSString*)package {
  return [self lookupKeyForAsset:asset fromPackage:package fromBundle:nil];
}

+ (NSString*)lookupKeyForAsset:(NSString*)asset
                   fromPackage:(NSString*)package
                    fromBundle:(nullable NSBundle*)bundle {
  return [self lookupKeyForAsset:[NSString stringWithFormat:@"packages/%@/%@", package, asset]
                      fromBundle:bundle];
}

@end
