// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERPLATFORMVIEWS_H_
#define FLUTTER_FLUTTERPLATFORMVIEWS_H_

#import <CoreMedia/CoreMedia.h>
#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>

#include "FlutterCodecs.h"
#include "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * An instance of a `UIView` created by a `FlutterPlatformViewFactory`.
 *
 * This protocol is used to associate a view with a dispose callback.
 */
FLUTTER_EXPORT
@protocol FlutterPlatformView <NSObject>
- (UIView*)view;
/**
 * Called when the Flutter engine no longer needs this `view`.
 *
 * The implementation of this method should release any kept for this view.
 */
- (void)dispose;
@end

FLUTTER_EXPORT
@protocol FlutterPlatformViewFactory <NSObject>
/**
 * Create a `FlutterPlatformView`.
 *
 * Implemented by iOS code that expose a `UIView` for embedding in a Flutter app.
 *
 * The implementation of this method should create a new `UIView` and return a `FlutterPlatformView`
 * that wraps it.
 *
 * @param frame The rectangle for the newly created `UIView` measued in points.
 * @param viewId A unique identifier for this `UIView`.
 * @param args Parameters for creating the `UIView` sent from the Dart side of the Flutter app.
 *   If `createArgsCodec` is not implemented, or if no creation arguments were sent from the Dart
 *   code, this will be null. Otherwise this will be the value sent from the Dart code as decoded by
 *   `createArgsCodec`.
 */
- (NSObject<FlutterPlatformView>*)createWithFrame:(CGRect)frame
                                           viewId:(int64_t)viewId
                                          andArgs:(id _Nullable)args;

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
