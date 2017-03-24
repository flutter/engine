// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERCHANNELS_H_
#define FLUTTER_FLUTTERCHANNELS_H_

#include "FlutterBinaryMessenger.h"
#include "FlutterCodecs.h"

typedef void (^FlutterReplyHandler)(id reply);
typedef void (^FlutterMessageHandler)(id message,
                                      FlutterReplyHandler replyHandler);

FLUTTER_EXPORT
@interface FlutterMessageChannel : NSObject
+ (instancetype)messageChannelNamed:(NSString*)name
                    binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                              codec:(NSObject<FlutterMessageCodec>*)codec;
- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMessageCodec>*)codec;
- (void)sendMessage:(id)message;
- (void)sendMessage:(id)message replyHandler:(FlutterReplyHandler)handler;
- (void)setMessageHandler:(FlutterMessageHandler)handler;
@end

typedef void (^FlutterResultReceiver)(id result);
typedef void (^FlutterMethodCallHandler)(FlutterMethodCall* call,
                                         FlutterResultReceiver resultReceiver);

FLUTTER_EXPORT
extern NSObject const* FlutterMethodNotImplemented;

FLUTTER_EXPORT
@interface FlutterMethodChannel : NSObject
+ (instancetype)methodChannelNamed:(NSString*)name
                   binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                             codec:(NSObject<FlutterMethodCodec>*)codec;
- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMethodCodec>*)codec;
- (void)invokeMethod:(NSString*)method arguments:(id)arguments;
- (void)invokeMethod:(NSString*)method
           arguments:(id)arguments
      resultReceiver:(FlutterResultReceiver)resultReceiver;
- (void)setMethodCallHandler:(FlutterMethodCallHandler)handler;
@end

typedef void (^FlutterEventReceiver)(id event);

FLUTTER_EXPORT
@protocol FlutterStreamHandler
- (void)onListenWithArguments:(id)arguments
                eventReceiver:(FlutterEventReceiver)eventReceiver
                        error:(FlutterError**)error;
- (void)onCancelWithArguments:(id)arguments
                        error:(FlutterError**)error;
@end

FLUTTER_EXPORT
extern NSObject const* FlutterEndOfEventStream;

FLUTTER_EXPORT
@interface FlutterEventChannel : NSObject
+ (instancetype)eventChannelNamed:(NSString*)name
                  binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                            codec:(NSObject<FlutterMethodCodec>*)codec;
- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMethodCodec>*)codec;
- (void)setStreamHandler:(NSObject<FlutterStreamHandler>*)streamHandler;
@end

#endif  // FLUTTER_FLUTTERCHANNELS_H_
