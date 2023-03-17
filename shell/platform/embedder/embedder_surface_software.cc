// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_surface_software.h"

#include <utility>

#include "flutter/fml/trace_event.h"

#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

EmbedderSurfaceSoftware::EmbedderSurfaceSoftware(EmbedderStudioSoftware* studio,
                                                 bool render_to_surface)
    : studio_(studio), render_to_surface_(render_to_surface) {}

EmbedderSurfaceSoftware::~EmbedderSurfaceSoftware() = default;

// |EmbedderSurface|
bool EmbedderSurfaceSoftware::IsValid() const {
  return true;
}

// |EmbedderSurface|
std::unique_ptr<Surface> EmbedderSurfaceSoftware::CreateGPUSurface() {
  if (!IsValid()) {
    return nullptr;
  }
  auto surface =
      std::make_unique<GPUSurfaceSoftware>(studio_, render_to_surface_);

  if (!surface->IsValid()) {
    return nullptr;
  }

  return surface;
}

// |EmbedderSurface|
sk_sp<GrDirectContext> EmbedderSurfaceSoftware::CreateResourceContext() const {
  return nullptr;
}

}  // namespace flutter
