// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/graphics/FlutterDarwinContextMetalImpeller.h"

#include "flutter/common/graphics/persistent_cache.h"
#include "flutter/fml/logging.h"
#include "flutter/impeller/entity/mtl/entity_shaders.h"
#include "flutter/impeller/renderer/backend/metal/context_mtl.h"
#include "flutter/shell/common/context_options.h"
#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"

FLUTTER_ASSERT_ARC

static std::shared_ptr<impeller::Context> CreateImpellerContext() {
  std::vector<std::shared_ptr<fml::Mapping>> shader_mappings = {
      std::make_shared<fml::NonOwnedMapping>(impeller_entity_shaders_data,
                                             impeller_entity_shaders_length),
  };
  auto context = impeller::ContextMTL::Create(shader_mappings, "Impeller Library");
  if (!context) {
    FML_LOG(ERROR) << "Could not create Metal Impeller Context.";
    return nullptr;
  }
  FML_LOG(ERROR) << "Using the Impeller rendering backend.";

  return context;
}

@implementation FlutterDarwinContextMetalImpeller

- (instancetype)init {
  self = [super init];
  if (self != nil) {
    _context = CreateImpellerContext();
    _device = impeller::ContextMTL::Cast(*_context).GetMTLDevice();

    if (!_device) {
      FML_DLOG(ERROR) << "Could not acquire Metal device.";
      return nil;
    }

    CVReturn cvReturn = CVMetalTextureCacheCreate(kCFAllocatorDefault,  // allocator
                                                  nil,      // cache attributes (nil default)
                                                  _device,  // metal device
                                                  nil,      // texture attributes (nil default)
                                                  &_textureCache  // [out] cache
    );
    if (cvReturn != kCVReturnSuccess) {
      FML_DLOG(ERROR) << "Could not create Metal texture cache.";
      return nil;
    }
  }
  return self;
}

- (void)dealloc {
  if (_textureCache) {
    CFRelease(_textureCache);
  }
}

- (FlutterDarwinExternalTextureMetal*)
    createExternalTextureWithIdentifier:(int64_t)textureID
                                texture:(NSObject<FlutterTexture>*)texture {
  return [[FlutterDarwinExternalTextureMetal alloc] initWithTextureCache:_textureCache
                                                               textureID:textureID
                                                                 texture:texture
                                                          enableImpeller:YES];
}

@end
