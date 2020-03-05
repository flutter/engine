// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_surface_metal.h"

#include "flutter/shell/gpu/gpu_surface_metal.h"
#include "flutter/shell/platform/darwin/ios/ios_context_metal.h"

namespace flutter {

static IOSContextMetal* CastToMetalContext(const std::shared_ptr<IOSContext>& context) {
  return reinterpret_cast<IOSContextMetal*>(context.get());
}

IOSSurfaceMetal::IOSSurfaceMetal(fml::scoped_nsobject<CAMetalLayer> layer,
                                 std::shared_ptr<IOSContext> context,
                                 FlutterPlatformViewsController* platform_views_controller)
    : IOSSurface(std::move(context), platform_views_controller), layer_(std::move(layer)) {
  if (!layer_) {
    return;
  }

  auto metal_context = CastToMetalContext(GetContext());

  layer_.get().device = metal_context->GetDevice().get();
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
std::unique_ptr<Surface> IOSSurfaceMetal::CreateGPUSurface(GrContext* /* unused */) {
  auto metal_context = CastToMetalContext(GetContext());

  return std::make_unique<GPUSurfaceMetal>(this,    // Metal surface delegate
                                           layer_,  // layer
                                           metal_context->GetMainContext(),      // context
                                           metal_context->GetMainCommandQueue()  // command queue
  );
}

SkRect IOSSurfaceMetal::GetPlatformViewRect(int view_id) {
  FlutterPlatformViewsController* platform_views_controller = GetPlatformViewsController();
  FML_CHECK(platform_views_controller != nullptr);
  return platform_views_controller->GetPlatformViewRect(view_id);
}

bool IOSSurfaceMetal::SubmitFrame(GrContext* context, SkCanvas* background_canvas) {
  FlutterPlatformViewsController* platform_views_controller = GetPlatformViewsController();
  if (platform_views_controller == nullptr) {
    return true;
  }

  bool submitted = platform_views_controller->SubmitFrame(context, nullptr, background_canvas);
  [CATransaction commit];
  return submitted;
}

}  // namespace flutter
