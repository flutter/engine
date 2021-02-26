// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyHandlerBase.h"

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterKeyHandlerBase.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterIntermediateKeyResponder.h"

@interface FlutterKeyboardManager : NSObject

- (nonnull instancetype)initWithOwner:(nonnull NSResponder*)weakOwner;

- (void)addHandler:(nonnull id<FlutterKeyHandlerBase>)handler;

- (void)addAdditionalHandler:(nonnull FlutterIntermediateKeyResponder*)handler;

- (void)keyDown:(nonnull NSEvent*)event;

- (void)keyUp:(nonnull NSEvent*)event;

- (void)flagsChanged:(nonnull NSEvent*)event;

@end
