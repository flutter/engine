// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_DARWIN_GRAPHICS_DARWIN_CONTEXT_METAL_H_
#define SHELL_PLATFORM_DARWIN_GRAPHICS_DARWIN_CONTEXT_METAL_H_

#import <Foundation/Foundation.h>
#include <Metal/Metal.h>
#include "third_party/skia/include/gpu/GrDirectContext.h"

NS_ASSUME_NONNULL_BEGIN

@interface FlutterDarwinContextMetal : NSObject

- (instancetype)init;

@property(nonatomic, readonly) id<MTLDevice> mtlDevice;
@property(nonatomic, readonly) id<MTLCommandQueue> mtlCommandQueue;
@property(nonatomic, readonly) sk_sp<GrDirectContext> mainContext;
@property(nonatomic, readonly) sk_sp<GrDirectContext> resourceContext;

@end

NS_ASSUME_NONNULL_END

#endif  // SHELL_PLATFORM_DARWIN_GRAPHICS_DARWIN_CONTEXT_METAL_H_
