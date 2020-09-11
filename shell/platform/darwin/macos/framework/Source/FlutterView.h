// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

@protocol FlutterViewDelegate <NSObject>
/**
 * Called when the view's backing store changes size.
 */
- (void)viewDidReshape:(nonnull NSView*)view;

- (void)scheduleOnRasterTread:(nonnull dispatch_block_t)block;

@end

/**
 * View capable of acting as a rendering target and input source for the Flutter
 * engine.
 */
@interface FlutterView : NSView

@property(readwrite, nonatomic, nonnull) NSOpenGLContext* openGLContext;

- (nullable instancetype)initWithFrame:(NSRect)frame
                          shareContext:(nonnull NSOpenGLContext*)shareContext
                              delegate:(nonnull id<FlutterViewDelegate>)delegate
    NS_DESIGNATED_INITIALIZER;

- (nullable instancetype)initWithShareContext:(nonnull NSOpenGLContext*)shareContext
                                     delegate:(nonnull id<FlutterViewDelegate>)delegate;

- (nullable instancetype)initWithFrame:(NSRect)frameRect
                           pixelFormat:(nullable NSOpenGLPixelFormat*)format NS_UNAVAILABLE;
- (nonnull instancetype)initWithFrame:(NSRect)frameRect NS_UNAVAILABLE;
- (nullable instancetype)initWithCoder:(nonnull NSCoder*)coder NS_UNAVAILABLE;
- (nonnull instancetype)init NS_UNAVAILABLE;

- (void)start;
- (void)present;
- (int)getFrameBufferIdForWidth:(int)width height:(int)height;

@end
