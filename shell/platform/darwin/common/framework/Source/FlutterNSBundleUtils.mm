// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Foundation/Foundation.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"

FLUTTER_ASSERT_ARC

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

NSBundle* FLTGetApplicationBundle() {
  NSBundle* mainBundle = [NSBundle mainBundle];
  // App extension bundle is in Runner.app/PlugIns/Extension.appex.
  if ([mainBundle.bundleURL.pathExtension isEqualToString:@"appex"]) {
    // Up two levels.
    return [NSBundle bundleWithURL:mainBundle.bundleURL.URLByDeletingLastPathComponent
                                       .URLByDeletingLastPathComponent];
  }
  return mainBundle;
}

NSBundle* FLTFrameworkBundleWithIdentifier(NSString* flutterBundleID) {
  NSBundle* appBundle = FLTGetApplicationBundle();
  NSBundle* bundle = FLTFrameworkBundleInternal(flutterBundleID, appBundle.privateFrameworksURL);
  if (bundle == nil) {
    // Fallback to slow implementation.
    bundle = [NSBundle bundleWithIdentifier:flutterBundleID];
  }
  if (bundle == nil) {
    bundle = [NSBundle mainBundle];
  }
  return bundle;
}

NSURL* FLTAssetsFromBundle(NSBundle* bundle) {
  NSString* assetsPathFromInfoPlist = [bundle objectForInfoDictionaryKey:@"FLTAssetsPath"];
  NSString* flutterAssetsName =
      assetsPathFromInfoPlist != nil ? assetsPathFromInfoPlist : @"flutter_assets";
  NSURL* assets = [bundle URLForResource:flutterAssetsName withExtension:nil];

  if ([assets checkResourceIsReachableAndReturnError:NULL]) {
    return assets;
  }
  return nil;
}
