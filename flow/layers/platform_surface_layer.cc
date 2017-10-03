// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/platform_surface_layer.h"
#include "flutter/common/threads.h"
#include "flutter/flow/platform_surface.h"
#include "lib/fxl/synchronization/waitable_event.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrContext.h"
#include "third_party/skia/include/gpu/GrTexture.h"
#include "third_party/skia/include/gpu/GrTypes.h"

namespace flow {

PlatformSurfaceLayer::PlatformSurfaceLayer() = default;

PlatformSurfaceLayer::~PlatformSurfaceLayer() = default;

void PlatformSurfaceLayer::Preroll(PrerollContext* context,
                                   const SkMatrix& matrix) {
  set_paint_bounds(SkRect::MakeXYWH(offset_.x(), offset_.y(), size_.width(),
                                    size_.height()));
}

void PlatformSurfaceLayer::Paint(PaintContext& context) {
  PlatformSurface* surface =
      context.platform_surface_registry.GetPlatformSurface(surface_id_);
  if (surface == nullptr) {
    FXL_DLOG(WARNING) << "No platform surface with id: " << surface_id_;
    return;
  }
  sk_sp<SkImage> sk_image =
      surface->MakeSkImage(paint_bounds().width(), paint_bounds().height(),
                           context.canvas.getGrContext());
  if (!sk_image) {
    return;
  }
  context.canvas.drawImage(sk_image, paint_bounds().x(), paint_bounds().y());
}

}  // namespace flow
