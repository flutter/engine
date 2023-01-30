// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/display_list/display_list_blend_mode.h"
#include "flutter/display_list/display_list_tile_mode.h"
#include "flutter/impeller/entity/entity.h"
#include "flutter/impeller/geometry/color.h"
#include "flutter/impeller/geometry/scalar.h"
#include "third_party/skia/include/core/SkColor.h"

namespace impeller {

BlendMode ToBlendMode(flutter::DlBlendMode mode);

Entity::TileMode ToTileMode(flutter::DlTileMode tile_mode);

Color ToColor(const SkColor& color);

}  // namespace impeller