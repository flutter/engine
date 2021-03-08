// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyHandler.h"

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyFinalResponder.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyHandler.h"

@interface FlutterKeyboardManager : NSObject

- (nonnull instancetype)initWithOwner:(nonnull NSResponder*)weakOwner;

- (void)addHandler:(nonnull id<FlutterKeyHandler>)handler;

- (void)addAdditionalHandler:(nonnull id<FlutterKeyFinalResponder>)handler;

- (void)handleEvent:(nonnull NSEvent*)event;

@end
