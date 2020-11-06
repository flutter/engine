// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/ios_surface_metal.h"

#include "flutter/shell/gpu/gpu_surface_metal.h"
#import "flutter/shell/platform/darwin/ios/ios_context_metal.h"

namespace flutter {

static const IOSContextMetal& CastToMetalContext(const IOSContext& context) {
  return static_cast<const IOSContextMetal&>(context);
}

IOSSurfaceMetal::IOSSurfaceMetal(fml::scoped_nsobject<CAMetalLayer> layer,
                                 const IOSContext& context)
    : IOSSurface(std::move(context)), layer_(std::move(layer)) {
  if (!layer_) {
    return;
  }

  auto& metal_context = CastToMetalContext(ios_context_);

  layer_.get().device = metal_context.GetDevice().get();
  layer_.get().presentsWithTransaction = YES;

  is_valid_ = true;
}

// |IOSSurface|
IOSSurfaceMetal::~IOSSurfaceMetal() = default;

// |IOSSurface|
bool IOSSurfaceMetal::IsValid() const {
  return is_valid_;
}

// |IOSSurface|
void IOSSurfaceMetal::UpdateStorageSizeIfNecessary() {
  // Nothing to do.
}

// |IOSSurface|
std::unique_ptr<Surface> IOSSurfaceMetal::CreateGPUSurface(GrDirectContext* /* unused */) {
  auto& metal_context = CastToMetalContext(ios_context_);

  return std::make_unique<GPUSurfaceMetal>(layer_,                              // layer
                                           metal_context.GetMainContext(),      // context
                                           metal_context.GetMainCommandQueue()  // command queue
  );
}

}  // namespace flutter
