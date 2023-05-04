// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>

namespace impeller {

/// Compute signed-distance field for an 8-bpp grayscale image (values greater
/// than 127 are considered "on") For details of this algorithm, see "The 'dead
/// reckoning' signed distance transform" [Grevera 2004]
void ConvertBitmapToSignedDistanceField(uint8_t* pixels,
                                        uint16_t width,
                                        uint16_t height);
}  // namespace impeller
