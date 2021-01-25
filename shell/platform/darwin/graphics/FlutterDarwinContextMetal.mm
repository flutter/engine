// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/graphics/FlutterDarwinContextMetal.h"

#include "flutter/common/graphics/persistent_cache.h"
#include "flutter/fml/logging.h"
#include "flutter/shell/gpu/gpu_binary_archive_metal.h"
#include "third_party/skia/include/gpu/GrContextOptions.h"
#include "third_party/skia/include/gpu/mtl/GrMtlBackendContext.h"

static GrContextOptions CreateMetalGrContextOptions() {
  GrContextOptions options = {};
  if (flutter::PersistentCache::cache_sksl()) {
    options.fShaderCacheStrategy = GrContextOptions::ShaderCacheStrategy::kSkSL;
  }
  flutter::PersistentCache::MarkStrategySet();
  options.fPersistentCache = flutter::PersistentCache::GetCacheForProcess();
  return options;
}

@implementation FlutterDarwinContextMetal {
  std::unique_ptr<flutter::GPUBinaryArchiveMetal> _binary_archive
      FLUTTER_METAL_BINARY_ARCHIVE_AVAILABLE;
}

- (instancetype)initWithDefaultMTLDevice {
  id<MTLDevice> device = MTLCreateSystemDefaultDevice();
  return [self initWithMTLDevice:device commandQueue:[device newCommandQueue]];
}

- (instancetype)initWithMTLDevice:(id<MTLDevice>)device
                     commandQueue:(id<MTLCommandQueue>)commandQueue {
  self = [super init];
  if (self != nil) {
    _device = device;

    if (!_device) {
      FML_DLOG(ERROR) << "Could not acquire Metal device.";
      [self release];
      return nil;
    }

    _commandQueue = commandQueue;

    if (!_commandQueue) {
      FML_DLOG(ERROR) << "Could not create Metal command queue.";
      [self release];
      return nil;
    }

    [_commandQueue setLabel:@"Flutter Main Queue"];

    if (@available(macos 11.0, ios 14.0, *)) {
      _binary_archive = std::make_unique<flutter::GPUBinaryArchiveMetal>(_device);
    }

    auto contextOptions = CreateMetalGrContextOptions();

    {
      GrMtlBackendContext context;
      context.fDevice.reset([_device retain]);
      context.fQueue.reset([_commandQueue retain]);
      if (@available(macos 11.0, ios 14.0, *)) {
        if (_binary_archive != nullptr) {
          context.fBinaryArchive = _binary_archive->GetBinaryArchiveHandle();
        }
      }

      _mainContext = GrDirectContext::MakeMetal(context, contextOptions);
    }

    {
      GrMtlBackendContext context;
      context.fDevice.reset([_device retain]);
      context.fQueue.reset([_commandQueue retain]);

      _resourceContext = GrDirectContext::MakeMetal(context, contextOptions);
    }

    if (!_mainContext || !_resourceContext) {
      FML_DLOG(ERROR) << "Could not create Skia Metal contexts.";
      [self release];
      return nil;
    }

    _resourceContext->setResourceCacheLimits(0u, 0u);
  }
  return self;
}

@end
