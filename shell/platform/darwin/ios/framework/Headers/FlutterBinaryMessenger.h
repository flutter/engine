// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERBINARYMESSAGES_H_
#define FLUTTER_FLUTTERBINARYMESSAGES_H_

#import <Foundation/Foundation.h>

#include "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN
typedef void (^FlutterBinaryReplyHandler)(NSData* _Nullable reply);
typedef void (^FlutterBinaryMessageHandler)(
    NSData* _Nullable message,
    FlutterBinaryReplyHandler replyHandler);

FLUTTER_EXPORT
@protocol FlutterBinaryMessenger<NSObject>
- (void)sendBinaryMessage:(NSData* _Nullable)message
              channelName:(NSString*)channelName;

- (void)sendBinaryMessage:(NSData* _Nullable)message
              channelName:(NSString*)channelName
       binaryReplyHandler:(FlutterBinaryReplyHandler)handler;

- (void)setBinaryMessageHandlerOnChannel:(NSString*)channelName
                    binaryMessageHandler:(FlutterBinaryMessageHandler _Nullable)handler;
@end
NS_ASSUME_NONNULL_END
#endif  // FLUTTER_FLUTTERBINARYMESSAGES_H_
