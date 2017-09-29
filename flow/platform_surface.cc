// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/platform_surface.h"

namespace flow {

static std::map<int, PlatformSurface*> mapping = {};

int PlatformSurface::RegisterPlatformSurface(PlatformSurface* surface) {
  int id = mapping.size() + 1;
  mapping[id] = surface;
  surface->id_ = id;
  return id;
}

void PlatformSurface::DisposePlatformSurface(int id) {
  PlatformSurface* surface = mapping[id];
  mapping.erase(id);
  delete surface;
}

PlatformSurface* PlatformSurface::GetPlatformSurface(int id) {
  return mapping[id];
}

}  // namespace flow
