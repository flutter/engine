// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <impeller/blending.glsl>
#include <impeller/color.glsl>
#include <impeller/types.glsl>

layout(constant_id = 0) const int blend_type = 0;

// kScreen = 0,
// kOverlay,
// kDarken,
// kLighten,
// kColorDodge,
// kColorBurn,
// kHardLight,
// kSoftLight,
// kDifference,
// kExclusion,
// kMultiply,
// kHue,
// kSaturation,
// kColor,
// kLuminosity,
f16vec3 Blend(f16vec3 dst, f16vec3 src) {
  switch (blend_type) {
    case 0:
      return IPBlendScreen(dst, src);
    case 1:
      return IPBlendOverlay(dst, src);
    case 2:
      return IPBlendDarken(dst, src);
    case 3:
      return IPBlendLighten(dst, src);
    case 4:
      return IPBlendColorDodge(dst, src);
    case 5:
      return IPBlendColorBurn(dst, src);
    case 6:
      return IPBlendHardLight(dst, src);
    case 7:
      return IPBlendSoftLight(dst, src);
    case 8:
      return IPBlendDifference(dst, src);
    case 9:
      return IPBlendExclusion(dst, src);
    case 10:
      return IPBlendMultiply(dst, src);
    case 11:
      return IPBlendHue(dst, src);
    case 12:
      return IPBlendSaturation(dst, src);
    case 13:
      return IPBlendColor(dst, src);
    case 14:
      return IPBlendLuminosity(dst, src);
    default:
      return f16vec3(0.0hf);
  }
}
