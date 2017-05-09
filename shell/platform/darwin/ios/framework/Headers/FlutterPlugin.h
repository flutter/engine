// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERPLUGIN_H_
#define FLUTTER_FLUTTERPLUGIN_H_

#import <UIKit/UIKit.h>

#include "FlutterBinaryMessenger.h"
#include "FlutterChannels.h"
#include "FlutterCodecs.h"

NS_ASSUME_NONNULL_BEGIN
@protocol FlutterPluginRegistrar;

/**
 iOS part of a Flutter plugin.

 Provides a protocol of optional callback methods and the means to register with
 the application.
 */
@protocol FlutterPlugin<NSObject>
@required
/**
 Registers this plugin.

 - Parameters:
   - registrar: a helper providing application context and methods for
     registering callbacks
 */
+ (void)registerWithRegistrar:(NSObject<FlutterPluginRegistrar>*)registrar;
@optional
/**
 Handles an incoming method call from the Dart side (optional).

 - Parameters:
   - call: the method call
   - result: a result callback
 */
- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result;
- (BOOL)application:(UIApplication*)application
    didFinishLaunchingWithOptions:(NSDictionary*)launchOptions;
- (void)applicationDidBecomeActive:(UIApplication*)application;
- (void)applicationWillResignActive:(UIApplication*)application;
- (void)applicationDidEnterBackground:(UIApplication*)application;
- (void)applicationWillEnterForeground:(UIApplication*)application;
- (void)applicationWillTerminate:(UIApplication*)application;
- (void)application:(UIApplication*)application
    didRegisterUserNotificationSettings:(UIUserNotificationSettings*)notificationSettings;
- (void)application:(UIApplication*)application
    didRegisterForRemoteNotificationsWithDeviceToken:(NSData*)deviceToken;
- (BOOL)application:(UIApplication*)application
    didReceiveRemoteNotification:(NSDictionary*)userInfo
          fetchCompletionHandler:(void (^)(UIBackgroundFetchResult result))completionHandler;
- (BOOL)application:(UIApplication*)application
      handleOpenURL:(NSURL*)url;
@end

/**
 Plugin registration context.
 */
@protocol FlutterPluginRegistrar<NSObject>
/**
 Returns a `FlutterBinaryMessenger`
 */
- (NSObject<FlutterBinaryMessenger>*)messenger;
- (void)publish:(NSObject*)value;
- (void)addMethodCallDelegate:(NSObject<FlutterPlugin>*)delegate
                      channel:(FlutterMethodChannel*)channel;
- (void)addApplicationDelegate:(NSObject<FlutterPlugin>*)delegate;
@end

/**
 A registry of Flutter iOS plugins.

 Plugins are identified by unique string keys. It is strongly advised to use
 inverted domain names such as `com.mycompany.myproject.MyPlugin` as keys
 to avoid clashes.
 */
@protocol FlutterPluginRegistry<NSObject>
/**
 Returns a registrar for registering a plugin.

 - Parameter pluginKey: The unique key identifying the plugin.
 */
- (NSObject<FlutterPluginRegistrar>*)registrarForPlugin:(NSString*)pluginKey;
/**
 Returns whether the specified plugin has been registered.

 - Parameter pluginKey: The unique key identifying the plugin.
 - Returns: `YES` if `registrarForPlugin` has been called with `pluginKey`.
 */
- (BOOL)hasPlugin:(NSString*)pluginKey;

/**
 Returns a value published by the specified plugin.

 - Parameter pluginKey: The unique key identifying the plugin.
 - Returns: An object published by the plugin, if any. Will be `NSNull` if
   nothing has been published. Will be `nil` if the plugin has not been
   registered.
 */
- (NSObject*)valuePublishedByPlugin:(NSString*)pluginKey;
@end

NS_ASSUME_NONNULL_END;

#endif  // FLUTTER_FLUTTERPLUGIN_H_
