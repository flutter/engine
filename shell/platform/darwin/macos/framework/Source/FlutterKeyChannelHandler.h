// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyHandlerBase.h"

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterChannels.h"

@interface FlutterKeyChannelHandler : NSObject <FlutterKeyHandlerBase>

- (nonnull instancetype)initWithChannel:(nonnull FlutterBasicMessageChannel*)channel;

@end
