// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERPLUGINS_H_
#define FLUTTER_FLUTTERPLUGINS_H_

#include "FlutterChannels.h"
#include "FlutterViewController.h"

NS_ASSUME_NONNULL_BEGIN
/**
 A base class for plugins that communicate with Flutter using asynchronous
 method invocations on a single logical channel.
 */
FLUTTER_EXPORT
@interface FlutterMethodPlugin : NSObject
/**
 View controller associated to this plugin.
 */
@property(strong, nonatomic, readonly) FlutterViewController* controller;

/**
 Method channel used for communication with Flutter.
 */
@property(strong, nonatomic, readonly) FlutterMethodChannel* channel;
- (instancetype)init NS_UNAVAILABLE;
/**
 Initializes the plugin with a channel name.

 Sub-classes are intended to provide the channel name, asking their clients to
 specify only the controller. `FlutterStandardMethodCodec` will be used as the
 channel codec.

 - Parameters:
   - controller: A `FlutterViewController`.
   - channelName: The name of the `FlutterMethodChannel` used by this plugin.
 */
- (instancetype)initWithController:(FlutterViewController*)controller
                       channelName:(NSString*)channelName;
/**
 Initializes the plugin.

 Sub-classes are intended to provide the channel, asking their clients to
 specify only the controller.

 - Parameters:
   - controller: A `FlutterViewController`.
   - channel: A `FlutterMethodChannel` to use for the communication.
*/
- (instancetype)initWithController:(FlutterViewController*)controller
                           channel:(FlutterMethodChannel*)channel
    NS_DESIGNATED_INITIALIZER;

/**
 Handles an incoming method invocation from Flutter.

 Sub-classes should override this method; the default implementation reports
 that the method was not implemented.

 - Parameters:
   - method: The name of the method.
   - arguments: Arguments for the invocation.
   - result: A callback for submitting the result of the invocation.
 */
- (void)handleMethodInvocation:(NSString*)method
                     arguments:(id _Nullable)arguments
                        result:(FlutterResultReceiver)result;
@end
NS_ASSUME_NONNULL_END

#endif  // FLUTTER_FLUTTERPLUGINS_H_
