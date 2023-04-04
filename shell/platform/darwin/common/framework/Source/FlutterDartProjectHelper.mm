// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <Foundation/Foundation.h>

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
NSBundle* FLTFrameworkBundleWithIdentifier(NSString* bundleID) {
  NSBundle* bundle = FLTFrameworkBundleInternal(bundleID, NSBundle.mainBundle.privateFrameworksURL);
  if (bundle != nil) {
    return bundle;
  }
  // Fallback to slow implementation.
  return [NSBundle bundleWithIdentifier:bundleID];
}
