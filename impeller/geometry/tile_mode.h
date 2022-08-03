// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

namespace impeller {

// An enum to define how to repeat, fold, or omit colors outside of the
// typically defined range of the source of the colors (such as the
// bounds of an image or the defining geoetry of a gradient).
enum class TileMode {
  // Replicate the edge color if the shader draws outside of its original
  // bounds.
  kClamp,

  // Repeat the shader's image horizontally and vertically (or both along and
  // perpendicular to a gradient's geometry).
  kRepeat,

  // Repeat the shader's image horizontally and vertically, alternating mirror
  // images so that adjacent images always seam.
  kMirror,

  // Render the shader's image pixels only within its original bounds. If the
  // shader draws outside of its original bounds, transparent black is drawn
  // instead.
  kDecal,
};

}  // namespace impeller
