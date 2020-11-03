// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

// Manages the IOSurfaces for FlutterView
@interface FlutterSurfaceManager : NSObject

- (instancetype)initWithLayer:(CALayer*)containingLayer
                openGLContext:(NSOpenGLContext*)opengLContext;

- (void)ensureSurfaceSize:(CGSize)size;
- (void)swapBuffers;

- (uint32_t)glFrameBufferId;

@end
