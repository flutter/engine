// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/graphics/FlutterDarwinContextMetal.h"

#include "flutter/common/graphics/persistent_cache.h"
#include "flutter/fml/logging.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"

namespace {

static GrContextOptions CreateMetalGrContextOptions() {
  GrContextOptions options = {};
  if (flutter::PersistentCache::cache_sksl()) {
    options.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;
  }
  flutter::PersistentCache::MarkStrategySet();
  options.fPersistentCache = flutter::PersistentCache::GetCacheForProcess();
  return options;
}

}  // namespace

@implementation FlutterDarwinContextMetal {
}

- (instancetype)initWithDefaultMTLDevice {
  return [self initWithMTLDevice:MTLCreateSystemDefaultDevice()];
}

- (instancetype)initWithMTLDevice:(id<MTLDevice>)mtlDevice {
  self = [super init];
  if (self != nil) {
    _mtlDevice = mtlDevice;

    if (!_mtlDevice) {
      FML_DLOG(ERROR) << "Could not acquire Metal device.";
      return nil;
    }

    _mtlCommandQueue = [_mtlDevice newCommandQueue];

    if (!_mtlCommandQueue) {
      FML_DLOG(ERROR) << "Could not create Metal command queue.";
      return nil;
    }

    [_mtlCommandQueue setLabel:@"Flutter Main Queue"];

    auto contextOptions = CreateMetalGrContextOptions();

    // Skia expect arguments to `MakeMetal` transfer ownership of the reference in for release later
    // when the GrDirectContext is collected.
    _mainContext =
        GrDirectContext::MakeMetal([_mtlDevice retain], [_mtlCommandQueue retain], contextOptions);
    _resourceContext =
        GrDirectContext::MakeMetal([_mtlDevice retain], [_mtlCommandQueue retain], contextOptions);

    if (!_mainContext || !_resourceContext) {
      FML_DLOG(ERROR) << "Could not create Skia Metal contexts.";
      return nil;
    }

    _resourceContext->setResourceCacheLimits(0u, 0u);
  }
  return self;
}

@end
