// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/platform_surface.h"

namespace flow {

PlatformSurfaceRegistry::PlatformSurfaceRegistry() {
  mapping_ = {};
}
PlatformSurfaceRegistry::~PlatformSurfaceRegistry() = default;

int PlatformSurfaceRegistry::RegisterPlatformSurface(PlatformSurface* surface) {
  ASSERT_IS_GPU_THREAD
  int id = mapping_.size() + 1;
  mapping_[id] = surface;
  surface->id_ = id;
  return id;
}

void PlatformSurfaceRegistry::DisposePlatformSurface(int id) {
  ASSERT_IS_GPU_THREAD
  PlatformSurface* surface = mapping_[id];
  mapping_.erase(id);
  delete surface;
}

void PlatformSurfaceRegistry::AttachAll() {
  ASSERT_IS_GPU_THREAD;
  for (auto& it : mapping_) {
    it.second->Attach();
  }
}

void PlatformSurfaceRegistry::DetachAll() {
  ASSERT_IS_GPU_THREAD;
  for (auto& it : mapping_) {
    it.second->Detach();
  }
}

PlatformSurface* PlatformSurfaceRegistry::GetPlatformSurface(int id) {
  ASSERT_IS_GPU_THREAD
  return mapping_[id];
}

PlatformSurface::~PlatformSurface() = default;

}  // namespace flow
