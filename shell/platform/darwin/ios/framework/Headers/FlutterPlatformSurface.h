// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLUTTERPlatformSurface_H_
#define FLUTTER_FLUTTERPlatformSurface_H_

#import <CoreMedia/CoreMedia.h>
#import <Foundation/Foundation.h>

#include "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN

FLUTTER_EXPORT
@protocol FlutterPlatformSurface<NSObject>
- (CVPixelBufferRef)getPixelBuffer;
@end

FLUTTER_EXPORT
@protocol FlutterPlatformSurfaceRegistry<NSObject>
- (NSUInteger)registerPlatformSurface:(NSObject<FlutterPlatformSurface>*)surface;
- (void)newPlatformSurfaceFrameAvailable:(NSUInteger)surfaceId;
- (void)unregisterPlatformSurface:(NSUInteger)surfaceId;
@end

NS_ASSUME_NONNULL_END

#endif  // FLUTTER_PlatformSurface_H_
