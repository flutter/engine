// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERPLATFORMVIEWS_H_
#define FLUTTER_FLUTTERPLATFORMVIEWS_H_

#import <AppKit/AppKit.h>

#import "FlutterCodecs.h"
#import "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * Wraps a `NSView` for embedding in the Flutter hierarchy
 */
@protocol FlutterPlatformView <NSObject>
/**
 * Returns a reference to the `NSView` that is wrapped by this `FlutterPlatformView`.
 *
 * It is recommended to return a cached view instance in this method.
 * Constructing and returning a new NSView instance in this method might cause undefined behavior.
 *
 * TODO(richardjcai): Prevent [FlutterPlatformView view] to be called multiple times
 * in a single frame.
 */
- (NSView*)view;
@end

FLUTTER_EXPORT
@protocol FlutterPlatformViewFactory <NSObject>
/**
 * Create a `FlutterPlatformView`.
 *
 * Implemented by MacOS code that expose a `FlutterPlatformView` for embedding in a Flutter app.
 *
 * The implementation of this method should create a new `FlutterPlatformView` and return it.
 *
 * @param frame The rectangle for the newly created `FlutterPlatformView` measured in points.
 * @param viewId A unique identifier for this `FlutterPlatformView`.
 * @param args Parameters for creating the `FlutterPlatformView` sent from the Dart side of the
 * Flutter app. If `createArgsCodec` is not implemented, or if no creation arguments were sent from
 * the Dart code, this will be null. Otherwise this will be the value sent from the Dart code as
 * decoded by `createArgsCodec`.
 */
- (NSObject<FlutterPlatformView>*)createWithFrame:(CGRect)frame
                                   viewIdentifier:(int64_t)viewId
                                        arguments:(id _Nullable)args;

/**
 * Returns the `FlutterMessageCodec` for decoding the args parameter of `createWithFrame`.
 *
 * Only needs to be implemented if `createWithFrame` needs an arguments parameter.
 */
@optional
- (NSObject<FlutterMessageCodec>*)createArgsCodec;
@end

NS_ASSUME_NONNULL_END

#endif  // FLUTTER_FLUTTERPLATFORMVIEWS_H_
