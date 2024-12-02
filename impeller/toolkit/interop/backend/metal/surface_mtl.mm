// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/interop/backend/metal/surface_mtl.h"

#include "impeller/renderer/backend/metal/surface_mtl.h"

namespace impeller::interop {

SurfaceMTL::SurfaceMTL(Context& context, void* metal_drawable)
    : Surface(context,
              impeller::SurfaceMTL::MakeFromMetalLayerDrawable(
                  context.GetContext(),
                  (__bridge id<CAMetalDrawable>)metal_drawable)) {}

SurfaceMTL::~SurfaceMTL() = default;

}  // namespace impeller::interop
