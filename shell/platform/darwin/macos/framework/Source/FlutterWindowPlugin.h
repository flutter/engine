// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <AppKit/AppKit.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterBinaryMessenger.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterPluginMacOS.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"

@interface FlutterWindowPlugin : NSObject <FlutterPlugin>

+ (void)registerWithRegistrar:(nonnull id<FlutterPluginRegistrar>)registrar
                       engine:(nonnull FlutterEngine*)engine;

@end
