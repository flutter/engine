// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DITHERING_GLSL_
#define DITHERING_GLSL_

#include <impeller/types.glsl>

/// The dithering rate, which is 1.0 / 64.0, or 0.015625.
const float kDitherRate = 1.0 / 64.0;

/// The dimensions of the dithering LUT.
const float kImgSize = 8;

/// Returns the closest color to the input color using 8x8 ordered dithering.
///
/// Ordered dithering divides the output into a grid of cells, and then assigns
/// a different threshold to each cell. The threshold is used to determine if
/// the color should be rounded up (white) or down (black).
//
/// This technique was chosen mostly because Skia also uses it:
/// https://github.com/google/skia/blob/f9de059517a6f58951510fc7af0cba21e13dd1a8/src/opts/SkRasterPipeline_opts.h#L1717
///
/// See also:
/// - https://en.wikipedia.org/wiki/Ordered_dithering
/// - https://surma.dev/things/ditherpunk/
/// - https://shader-tutorial.dev/advanced/color-banding-dithering/
#define IPOrderedDither8x8(output, coords, lut)                           \
  {                                                                       \
    float value = texture(lut, coords / kImgSize).r - 0.5;                \
    output = vec4(clamp(output.rgb + value * kDitherRate, 0.0, output.a), \
                  output.a);                                              \
  }

#endif
