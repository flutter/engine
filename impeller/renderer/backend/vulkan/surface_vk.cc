// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/surface_vk.h"
#include "fml/logging.h"
#include "impeller/renderer/surface.h"

namespace impeller {

SurfaceVK::SurfaceVK(RenderTarget target, vk::UniqueSurfaceKHR surface)
    : Surface(target), surface_(std::move(surface)) {
  FML_DCHECK(surface_);
}

vk::SurfaceKHR SurfaceVK::GetSurface() const {
  return surface_.get();
}

SurfaceVK::~SurfaceVK() = default;

bool SurfaceVK::Present() const {
  FML_UNREACHABLE();
}

}  // namespace impeller
