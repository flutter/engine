// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <AppKit/AppKit.h>

#import "flutter/shell/platform/embedder/embedder.h"

@class FlutterEngine;
@class FlutterView;

/**
 */
@interface FlutterMouseState : NSObject

- (nullable instancetype)initWithEngine:(nonnull __weak FlutterEngine*)weakEngine;

- (void)mouseEntered:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)mouseExited:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)mouseDown:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)mouseUp:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)mouseDragged:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)rightMouseDown:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)rightMouseUp:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)rightMouseDragged:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)otherMouseDown:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)otherMouseUp:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)otherMouseDragged:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)mouseMoved:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)scrollWheel:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)magnifyWithEvent:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)rotateWithEvent:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

- (void)touchesBeganWithEvent:(nonnull NSEvent*)event inView:(nonnull FlutterView*)flutterView;

@end
