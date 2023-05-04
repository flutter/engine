// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/typographer/backends/skia/text_sdf_atlas.h"

#include <vector>

#include "impeller/geometry/point.h"
#include "impeller/geometry/scalar.h"

namespace impeller {

void ConvertBitmapToSignedDistanceField(uint8_t* pixels,
                                        uint16_t width,
                                        uint16_t height) {
  if (!pixels || width == 0 || height == 0) {
    return;
  }

  using ShortPoint = TPoint<uint16_t>;

  // distance to nearest boundary point map
  std::vector<Scalar> distance_map(width * height);
  // nearest boundary point map
  std::vector<ShortPoint> boundary_point_map(width * height);

  // Some helpers for manipulating the above arrays
#define image(_x, _y) (pixels[(_y)*width + (_x)] > 0x7f)
#define distance(_x, _y) distance_map[(_y)*width + (_x)]
#define nearestpt(_x, _y) boundary_point_map[(_y)*width + (_x)]

  const Scalar maxDist = hypot(width, height);
  const Scalar distUnit = 1;
  const Scalar distDiag = sqrt(2);

  // Initialization phase: set all distances to "infinity"; zero out nearest
  // boundary point map
  for (uint16_t y = 0; y < height; ++y) {
    for (uint16_t x = 0; x < width; ++x) {
      distance(x, y) = maxDist;
      nearestpt(x, y) = ShortPoint{0, 0};
    }
  }

  // Immediate interior/exterior phase: mark all points along the boundary as
  // such
  for (uint16_t y = 1; y < height - 1; ++y) {
    for (uint16_t x = 1; x < width - 1; ++x) {
      bool inside = image(x, y);
      if (image(x - 1, y) != inside || image(x + 1, y) != inside ||
          image(x, y - 1) != inside || image(x, y + 1) != inside) {
        distance(x, y) = 0;
        nearestpt(x, y) = ShortPoint{x, y};
      }
    }
  }

  // Forward dead-reckoning pass
  for (uint16_t y = 1; y < height - 2; ++y) {
    for (uint16_t x = 1; x < width - 2; ++x) {
      if (distance_map[(y - 1) * width + (x - 1)] + distDiag < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x - 1, y - 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x, y - 1) + distUnit < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x, y - 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x + 1, y - 1) + distDiag < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x + 1, y - 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x - 1, y) + distUnit < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x - 1, y);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
    }
  }

  // Backward dead-reckoning pass
  for (uint16_t y = height - 2; y >= 1; --y) {
    for (uint16_t x = width - 2; x >= 1; --x) {
      if (distance(x + 1, y) + distUnit < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x + 1, y);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x - 1, y + 1) + distDiag < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x - 1, y + 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x, y + 1) + distUnit < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x, y + 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
      if (distance(x + 1, y + 1) + distDiag < distance(x, y)) {
        nearestpt(x, y) = nearestpt(x + 1, y + 1);
        distance(x, y) = hypot(x - nearestpt(x, y).x, y - nearestpt(x, y).y);
      }
    }
  }

  // Interior distance negation pass; distances outside the figure are
  // considered negative
  // Also does final quantization.
  for (uint16_t y = 0; y < height; ++y) {
    for (uint16_t x = 0; x < width; ++x) {
      if (!image(x, y)) {
        distance(x, y) = -distance(x, y);
      }

      float norm_factor = 13.5;
      float dist = distance(x, y);
      float clamped_dist = fmax(-norm_factor, fmin(dist, norm_factor));
      float scaled_dist = clamped_dist / norm_factor;
      uint8_t quantized_value = ((scaled_dist + 1) / 2) * UINT8_MAX;
      pixels[y * width + x] = quantized_value;
    }
  }

#undef image
#undef distance
#undef nearestpt
}

}  // namespace impeller
