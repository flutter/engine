// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"

@interface TestFlutterPlatformView : NSView

/// Arguments passed via the params value in the create method call.
@property(nonatomic, copy) id args;

@end

@interface TestFlutterPlatformViewFactory : NSObject <FlutterPlatformViewFactory>
@end
