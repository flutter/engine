// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterAppDelegate.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterPluginAppLifeCycleDelegate.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterViewController.h"

@implementation FlutterAppDelegate {
  FlutterPluginAppLifeCycleDelegate* _lifeCycleDelegate;
}

- (instancetype)init {
  if (self = [super init]) {
    _lifeCycleDelegate = [[FlutterPluginAppLifeCycleDelegate alloc] init];
  }
  return self;
}

- (void)dealloc {
  [_lifeCycleDelegate release];
  [super dealloc];
}

- (BOOL)respondsToSelector:(SEL)selector {
  return [super respondsToSelector:selector] ||
         [_lifeCycleDelegate hasPluginThatRespondsToSelector:selector];
}

- (id)forwardingTargetForSelector:(SEL)selector {
  if ([_lifeCycleDelegate hasPluginThatRespondsToSelector:selector]) {
    return _lifeCycleDelegate;
  }
  return [super forwardingTargetForSelector:selector];
}

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
  return [_lifeCycleDelegate application:application didFinishLaunchingWithOptions:launchOptions];
}

// Returns the key window's rootViewController, if it's a FlutterViewController.
// Otherwise, returns nil.
- (FlutterViewController*)rootFlutterViewController {
  UIViewController* viewController = [UIApplication sharedApplication].keyWindow.rootViewController;
  if ([viewController isKindOfClass:[FlutterViewController class]]) {
    return (FlutterViewController*)viewController;
  }
  return nil;
}

- (void)touchesBegan:(NSSet*)touches withEvent:(UIEvent*)event {
  [super touchesBegan:touches withEvent:event];

  // Pass status bar taps to key window Flutter rootViewController.
  if (self.rootFlutterViewController != nil) {
    [self.rootFlutterViewController handleStatusBarTouches:event];
  }
}

#pragma mark - FlutterPluginRegistry methods. All delegating to the rootViewController

- (NSObject<FlutterPluginRegistrar>*)registrarForPlugin:(NSString*)pluginKey {
  UIViewController* rootViewController = _window.rootViewController;
  if ([rootViewController isKindOfClass:[FlutterViewController class]]) {
    return
        [[(FlutterViewController*)rootViewController pluginRegistry] registrarForPlugin:pluginKey];
  }
  return nil;
}

- (BOOL)hasPlugin:(NSString*)pluginKey {
  UIViewController* rootViewController = _window.rootViewController;
  if ([rootViewController isKindOfClass:[FlutterViewController class]]) {
    return [[(FlutterViewController*)rootViewController pluginRegistry] hasPlugin:pluginKey];
  }
  return false;
}

- (NSObject*)valuePublishedByPlugin:(NSString*)pluginKey {
  UIViewController* rootViewController = _window.rootViewController;
  if ([rootViewController isKindOfClass:[FlutterViewController class]]) {
    return [[(FlutterViewController*)rootViewController pluginRegistry]
        valuePublishedByPlugin:pluginKey];
  }
  return nil;
}

#pragma mark - FlutterAppLifeCycleProvider methods

- (void)addApplicationLifeCycleDelegate:(NSObject<FlutterPlugin>*)delegate {
  [_lifeCycleDelegate addDelegate:delegate];
}

@end
