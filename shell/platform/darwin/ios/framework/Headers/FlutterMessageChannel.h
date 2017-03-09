// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERMESSAGECHANNEL_H_
#define FLUTTER_FLUTTERMESSAGECHANNEL_H_

#import <Foundation/Foundation.h>

#include "FlutterMacros.h"
#include "FlutterMessageCodec.h"
#include "FlutterViewController.h"

typedef void (^FlutterReplyHandler)(id reply);
typedef void (^FlutterMessageHandler)(id message, FlutterReplyHandler replyHandler);

FLUTTER_EXPORT
@interface FlutterMessageChannel : NSObject
+ (instancetype) withController:(FlutterViewController*)controller
                           name:(NSString*)name
                          codec:(NSObject<FlutterMessageCodec>*)codec;
- (void) send:(id)message andHandleReplyWith:(FlutterReplyHandler)handler;
- (void) handleMessagesWith:(FlutterMessageHandler)handler;
@end

#endif  // FLUTTER_FLUTTERMESSAGECHANNEL_H_
