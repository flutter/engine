// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_SHADOW_UTIL_H_
#define UI_GFX_SHADOW_UTIL_H_

#include "ui/gfx/gfx_export.h"
#include "ui/gfx/image/image_skia.h"
#include "ui/gfx/shadow_value.h"

namespace gfx {

// A struct that describes a vector of shadows and their depiction as an image
// suitable for ninebox tiling.
struct GFX_EXPORT ShadowDetails {
  ShadowDetails();
  ShadowDetails(const ShadowDetails& other);
  ~ShadowDetails();

  // Returns a cached ShadowDetails for the given elevation (which controls
  // style) and corner radius. Creates the ShadowDetails first if necessary.
  static const ShadowDetails& Get(int elevation, int radius);

  // Description of the shadows.
  gfx::ShadowValues values;
  // Cached ninebox image based on |values|.
  gfx::ImageSkia ninebox_image;
};

}  // namespace gfx

#endif  // UI_GFX_SHADOW_UTIL_H_
