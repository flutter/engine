// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterAppDelegate.h"
#include <AppKit/AppKit.h>
#import "flutter/fml/logging.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterAppDelegate_internal.h"
#import "flutter/shell/platform/embedder/embedder.h"

@interface FlutterAppDelegate ()

/**
 * Returns the display name of the application as set in the Info.plist.
 */
- (NSString*)applicationName;

@end

@implementation FlutterAppDelegate {
  FlutterEngineTerminationHandler* _terminationHandler;
}

- (instancetype)init {
  if (self = [super init]) {
    _terminationHandler = nil;
  }
  return self;
}

- (void)applicationWillFinishLaunching:(NSNotification*)notification {
  // Update UI elements to match the application name.
  NSString* applicationName = [self applicationName];
  _mainFlutterWindow.title = applicationName;
  for (NSMenuItem* menuItem in _applicationMenu.itemArray) {
    menuItem.title = [menuItem.title stringByReplacingOccurrencesOfString:@"APP_NAME"
                                                               withString:applicationName];
  }
}

// This always returns NSTerminateNow, since by the time we get here, the
// application has already been asked if it should terminate or not, and if not,
// then termination never gets this far.
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender {
  return NSTerminateNow;
}

#pragma mark Private Methods

- (NSString*)applicationName {
  NSString* applicationName =
      [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleDisplayName"];
  if (!applicationName) {
    applicationName = [NSBundle.mainBundle objectForInfoDictionaryKey:@"CFBundleName"];
  }
  return applicationName;
}

- (void)setTerminationRequestHandler:(FlutterEngineTerminationHandler*)handler {
  _terminationHandler = handler;
}

- (void)tryToTerminateApplication:(FlutterApplication*)application
                         exitType:(FlutterAppExitType)type {
  if (_terminationHandler) {
    [_terminationHandler tryToTerminateApplication:application exitType:type result:nil];
  } else {
    [application terminateApplication:application];
  }
}

@end
