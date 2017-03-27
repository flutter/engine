// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERCHANNELS_H_
#define FLUTTER_FLUTTERCHANNELS_H_

#include "FlutterBinaryMessenger.h"
#include "FlutterCodecs.h"

NS_ASSUME_NONNULL_BEGIN
typedef void (^FlutterReplyHandler)(id reply);
typedef void (^FlutterMessageHandler)(id message,
                                      FlutterReplyHandler _Nullable replyHandler);

FLUTTER_EXPORT
@interface FlutterMessageChannel : NSObject
+ (instancetype)messageChannelWithName:(NSString*)name
                       binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger;
+ (instancetype)messageChannelWithName:(NSString*)name
                       binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                                 codec:(NSObject<FlutterMessageCodec>*)codec;
- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMessageCodec>*)codec;
- (void)sendMessage:(id _Nullable)message;
- (void)sendMessage:(id _Nullable)message replyHandler:(FlutterReplyHandler _Nullable)handler;
- (void)setMessageHandler:(FlutterMessageHandler _Nullable)handler;
@end

typedef void (^FlutterResultReceiver)(id _Nullable result);
typedef void (^FlutterMethodCallHandler)(FlutterMethodCall* call,
                                         FlutterResultReceiver resultReceiver);

FLUTTER_EXPORT
extern NSObject const* FlutterMethodNotImplemented;

FLUTTER_EXPORT
@interface FlutterMethodChannel : NSObject
+ (instancetype)methodChannelWithName:(NSString*)name
                      binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger;
+ (instancetype)methodChannelWithName:(NSString*)name
                      binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                             codec:(NSObject<FlutterMethodCodec>*)codec;
- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMethodCodec>*)codec;
- (void)invokeMethod:(NSString*)method arguments:(id _Nullable)arguments;
- (void)invokeMethod:(NSString*)method
           arguments:(id _Nullable)arguments
      resultReceiver:(FlutterResultReceiver _Nullable)resultReceiver;
- (void)setMethodCallHandler:(FlutterMethodCallHandler _Nullable)handler;
@end

typedef void (^FlutterEventReceiver)(id _Nullable event);

FLUTTER_EXPORT
@protocol FlutterStreamHandler
- (FlutterError* _Nullable)onListenWithArguments:(id _Nullable)arguments
                eventReceiver:(FlutterEventReceiver)eventReceiver;
- (FlutterError* _Nullable)onCancelWithArguments:(id _Nullable)arguments;
@end

FLUTTER_EXPORT
extern NSObject const* FlutterEndOfEventStream;

FLUTTER_EXPORT
@interface FlutterEventChannel : NSObject
+ (instancetype)eventChannelWithName:(NSString*)name
                  binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger;
+ (instancetype)eventChannelWithName:(NSString*)name
                  binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                            codec:(NSObject<FlutterMethodCodec>*)codec;
- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMethodCodec>*)codec;
- (void)setStreamHandler:(NSObject<FlutterStreamHandler>* _Nullable)streamHandler;
@end
NS_ASSUME_NONNULL_END

#endif  // FLUTTER_FLUTTERCHANNELS_H_
