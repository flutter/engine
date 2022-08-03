// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

const float kTileModeClamp = 0;
const float kTileModeRepeat = 1;
const float kTileModeMirror = 2;
const float kTileModeDecal = 3;

float GetInterpolantValue(float t, float tile_mode) {
  if (tile_mode == kTileModeClamp) {
    t = clamp(t, 0.0, 1.0);
  } else if (tile_mode == kTileModeRepeat) {
    t = fract(t);
  } else if (tile_mode == kTileModeMirror) {
    float t1 = t - 1;
    float t2 = t1 - 2 * floor(t1 * 0.5) - 1;
    t = abs(t2);    
  }
  return t;
}
