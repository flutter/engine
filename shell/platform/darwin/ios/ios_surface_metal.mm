// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/ios_surface_metal.h"
#include "flutter/shell/gpu/gpu_surface_metal.h"

namespace flutter {

IOSSurfaceMetal::IOSSurfaceMetal(fml::scoped_nsobject<CAMetalLayer> layer,
                                 FlutterPlatformViewsController* platform_views_controller)
    : IOSSurface(platform_views_controller), layer_(std::move(layer)) {}

IOSSurfaceMetal::~IOSSurfaceMetal() = default;

// |IOSSurface|
bool IOSSurfaceMetal::IsValid() const {
  return layer_;
}

// |IOSSurface|
bool IOSSurfaceMetal::ResourceContextMakeCurrent() {
  return false;
}

// |IOSSurface|
void IOSSurfaceMetal::UpdateStorageSizeIfNecessary() {}

// |IOSSurface|
std::unique_ptr<Surface> IOSSurfaceMetal::CreateGPUSurface() {
  return std::make_unique<GPUSurfaceMetal>(layer_);
}

flutter::ExternalViewEmbedder* IOSSurfaceGL::GetExternalViewEmbedder() {
  if (IsIosEmbeddedViewsPreviewEnabled()) {
    return this;
  } else {
    return nullptr;
  }
}

void IOSSurfaceMetal::BeginFrame(SkISize frame_size) {
  FlutterPlatformViewsController* platform_views_controller = GetPlatformViewsController();
  FML_CHECK(platform_views_controller != nullptr);
  platform_views_controller->SetFrameSize(frame_size);
  [CATransaction begin];
}

void IOSSurfaceMetal::PrerollCompositeEmbeddedView(int view_id) {
  FlutterPlatformViewsController* platform_views_controller = GetPlatformViewsController();
  FML_CHECK(platform_views_controller != nullptr);
  platform_views_controller->PrerollCompositeEmbeddedView(view_id);
}

std::vector<SkCanvas*> IOSSurfaceMetal::GetCurrentCanvases() {
  FlutterPlatformViewsController* platform_views_controller = GetPlatformViewsController();
  FML_CHECK(platform_views_controller != nullptr);
  return platform_views_controller->GetCurrentCanvases();
}

SkCanvas* IOSSurfaceMetal::CompositeEmbeddedView(int view_id,
                                                 const flutter::EmbeddedViewParams& params) {
  FlutterPlatformViewsController* platform_views_controller = GetPlatformViewsController();
  FML_CHECK(platform_views_controller != nullptr);
  return platform_views_controller->CompositeEmbeddedView(view_id, params);
}

bool IOSSurfaceMetal::SubmitFrame(GrContext* context) {
  FlutterPlatformViewsController* platform_views_controller = GetPlatformViewsController();
  if (platform_views_controller == nullptr) {
    return true;
  }

  bool submitted = platform_views_controller->SubmitFrame(true, std::move(context), context_);
  [CATransaction commit];
  return submitted;
}

}  // namespace flutter
