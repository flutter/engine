// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <AppKit/AppKit.h>

#import "flutter/shell/platform/embedder/embedder.h"

typedef void (^SendPointerEventCallback) (const FlutterPointerEvent&);

/**
 */
@interface FlutterMouseState : NSObject

- (nullable instancetype)initWithCallback:(nonnull SendPointerEventCallback)callback;

- (void)mouseEntered:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)mouseExited:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)mouseDown:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)mouseUp:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)mouseDragged:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)rightMouseDown:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)rightMouseUp:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)rightMouseDragged:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)otherMouseDown:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)otherMouseUp:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)otherMouseDragged:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)mouseMoved:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)scrollWheel:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)magnifyWithEvent:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)rotateWithEvent:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

- (void)touchesBeganWithEvent:(nonnull NSEvent*)event inView:(nonnull NSView*)flutterView;

@end
