// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERMETHODCHANNEL_H_
#define FLUTTER_FLUTTERMETHODCHANNEL_H_

#import <Foundation/Foundation.h>
#include "FlutterMethodCodec.h"
#include "FlutterMacros.h"
#include "FlutterViewController.h"

typedef void (^FlutterResultReceiver)(id successResult, FlutterError* errorResult);
typedef void (^FlutterEventReceiver)(id successEvent, FlutterError* errorEvent, BOOL done);
typedef void (^FlutterMethodCallHandler)(FlutterMethodCall* call, FlutterResultReceiver resultReceiver);
typedef void (^FlutterStreamHandler)(FlutterMethodCall* call, FlutterResultReceiver resultReceiver, FlutterEventReceiver eventReceiver);

FLUTTER_EXPORT
@interface FlutterMethodChannel : NSObject
+ (instancetype)withController:(FlutterViewController*)controller
                          name:(NSString*)name
                         codec:(NSObject<FlutterMethodCodec>*)codec;
- (void) handleMethodCallsWith:(FlutterMethodCallHandler)handler;
- (void) handleStreamWith:(FlutterStreamHandler)handler;
@end

#endif  // FLUTTER_FLUTTERMETHODCHANNEL_H_
