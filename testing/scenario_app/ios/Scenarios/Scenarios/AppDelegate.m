// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "AppDelegate.h"
#import "FlutterEngine+ScenariosTest.h"
#import "ScreenBeforeFlutter.h"
#import "TextPlatformView.h"

@interface NoStatusBarFlutterViewController : FlutterViewController

@end

@implementation NoStatusBarFlutterViewController
- (BOOL)prefersStatusBarHidden {
  return YES;
}
@end

@implementation AppDelegate

- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions {
  self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
  self.window.rootViewController = [AppDelegate rootViewController];
  [self.window makeKeyAndVisible];
  return [super application:application didFinishLaunchingWithOptions:launchOptions];
}

+ (UIViewController*)rootViewController {
  NSDictionary<NSString*, NSString*>* launchArgsMap = @{
    @"--platform-view" : @"platform_view",
    @"--platform-view-multiple" : @"platform_view_multiple",
    @"--platform-view-multiple-background-foreground" :
        @"platform_view_multiple_background_foreground",
    @"--platform-view-cliprect" : @"platform_view_cliprect",
    @"--platform-view-cliprrect" : @"platform_view_cliprrect",
    @"--platform-view-clippath" : @"platform_view_clippath",
    @"--platform-view-transform" : @"platform_view_transform",
    @"--platform-view-opacity" : @"platform_view_opacity",
    @"--platform-view-rotate" : @"platform_view_rotate",
  };
  __block NSString* goldenTestName = nil;
  [launchArgsMap
      enumerateKeysAndObjectsUsingBlock:^(NSString* argument, NSString* testName, BOOL* stop) {
        if ([[[NSProcessInfo processInfo] arguments] containsObject:argument]) {
          goldenTestName = testName;
          *stop = YES;
        }
      }];

  if (goldenTestName) {
    return [AppDelegate readyContextForPlatformViewTests:goldenTestName];
  } else if ([[[NSProcessInfo processInfo] arguments] containsObject:@"--screen-before-flutter"]) {
    return [[ScreenBeforeFlutter alloc] initWithEngineRunCompletion:nil];
  } else {
    return [[UIViewController alloc] init];
  }
}

+ (FlutterViewController*)readyContextForPlatformViewTests:(NSString*)scenarioIdentifier {
  FlutterEngine* engine = [[FlutterEngine alloc] initWithName:@"PlatformViewTest" project:nil];
  [engine runWithEntrypoint:nil];

  FlutterViewController* flutterViewController =
      [[NoStatusBarFlutterViewController alloc] initWithEngine:engine nibName:nil bundle:nil];
  [engine.binaryMessenger
      setMessageHandlerOnChannel:@"scenario_status"
            binaryMessageHandler:^(NSData* _Nullable message, FlutterBinaryReply _Nonnull reply) {
              [engine.binaryMessenger
                  sendOnChannel:@"set_scenario"
                        message:[scenarioIdentifier dataUsingEncoding:NSUTF8StringEncoding]];
            }];
  TextPlatformViewFactory* textPlatformViewFactory =
      [[TextPlatformViewFactory alloc] initWithMessenger:flutterViewController.binaryMessenger];
  NSObject<FlutterPluginRegistrar>* registrar =
      [flutterViewController.engine registrarForPlugin:@"scenarios/TextPlatformViewPlugin"];
  [registrar registerViewFactory:textPlatformViewFactory withId:@"scenarios/textPlatformView"];
  return flutterViewController;
}

@end
