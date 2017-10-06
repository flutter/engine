// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/platform_surface.h"

namespace flow {

PlatformSurfaceRegistry::PlatformSurfaceRegistry() = default;

PlatformSurfaceRegistry::~PlatformSurfaceRegistry() = default;

size_t PlatformSurfaceRegistry::RegisterPlatformSurface(std::shared_ptr<PlatformSurface> surface) {
  ASSERT_IS_GPU_THREAD
  size_t id = counter_++;
  mapping_[id] = surface;
  surface->id_ = id;
  return id;
}

void PlatformSurfaceRegistry::UnregisterPlatformSurface(size_t id) {
  ASSERT_IS_GPU_THREAD
  mapping_.erase(id);
}

void PlatformSurfaceRegistry::OnGrContextCreated() {
  ASSERT_IS_GPU_THREAD;
  for (auto& it : mapping_) {
    it.second->OnGrContextCreated();
  }
}

void PlatformSurfaceRegistry::OnGrContextDestroyed() {
  ASSERT_IS_GPU_THREAD;
  for (auto& it : mapping_) {
    it.second->OnGrContextDestroyed();
  }
}

std::shared_ptr<PlatformSurface> PlatformSurfaceRegistry::GetPlatformSurface(size_t id) {
  ASSERT_IS_GPU_THREAD
  return mapping_[id];
}

PlatformSurface::~PlatformSurface() = default;

}  // namespace flow
