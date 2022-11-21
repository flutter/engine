// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TEXTURE_GLSL_
#define TEXTURE_GLSL_

#include <impeller/types.glsl>
#include <impeller/branching.glsl>

/// Sample from a texture.
///
/// If `y_coord_scale` < 0.0, the Y coordinate is flipped. This is useful
/// for Impeller graphics backends that use a flipped framebuffer coordinate
/// space.
f16vec4 IPSample(sampler2D texture_sampler, f16vec2 coords, float16_t y_coord_scale) {
  if (y_coord_scale < 0.0hf) {
    coords.y = 1.0hf - coords.y;
  }
  return f16vec4(texture(texture_sampler, coords));
}

/// Sample from a texture.
///
/// If `y_coord_scale` < 0.0, the Y coordinate is flipped. This is useful
/// for Impeller graphics backends that use a flipped framebuffer coordinate
/// space.
/// The range of `coods` will be mapped from [0, 1] to [half_texel, 1 - half_texel]
f16vec4 IPSampleLinear(sampler2D texture_sampler, f16vec2 coords, float16_t y_coord_scale, f16vec2 half_texel) {
  coords.x = mix(half_texel.x, 1.0hf - half_texel.x, coords.x);
  coords.y = mix(half_texel.y, 1.0hf - half_texel.y, coords.y);
  return IPSample(texture_sampler, coords, y_coord_scale);
}

// These values must correspond to the order of the items in the
// 'Entity::TileMode' enum class.
const float kTileModeClamp = 0;
const float kTileModeRepeat = 1;
const float kTileModeMirror = 2;
const float kTileModeDecal = 3;

/// Remap a float16_t using a tiling mode.
///
/// When `tile_mode` is `kTileModeDecal`, no tiling is applied and `t` is
/// returned. In all other cases, a value between 0 and 1 is returned by tiling
/// `t`.
/// When `t` is between [0 to 1), the original unchanged `t` is always returned.
float16_t IPFloatTile(float16_t t, float16_t tile_mode) {
  if (tile_mode == float16_t(kTileModeClamp)) {
    t = clamp(t, 0.0hf, 1.0hf);
  } else if (tile_mode == float16_t(kTileModeRepeat)) {
    t = fract(t);
  } else if (tile_mode == float16_t(kTileModeMirror))  {
    float16_t t1 = t - 1.0hf;
    float16_t t2 = t1 - 2.0hf * floor(t1 * 0.5hf) - 1.0hf;
    t = abs(t2);
  }
  return t;
}

/// Remap a vec2 using a tiling mode.
///
/// Runs each component of the vec2 through `IPFloatTile`.
f16vec2 IPVec2Tile(f16vec2 coords, float16_t x_tile_mode, float16_t y_tile_mode) {
  return f16vec2(IPFloatTile(coords.x, x_tile_mode),
              IPFloatTile(coords.y, y_tile_mode));
}

/// Sample a texture, emulating a specific tile mode.
///
/// This is useful for Impeller graphics backend that don't have native support
/// for Decal.
f16vec4 IPSampleWithTileMode(sampler2D tex,
                          f16vec2 coords,
                          float16_t y_coord_scale,
                          float16_t x_tile_mode,
                          float16_t y_tile_mode) {
  if (x_tile_mode == kTileModeDecal && (coords.x < 0.hf || coords.x >= 1.hf) ||
      y_tile_mode == kTileModeDecal && (coords.y < 0.hf || coords.y >= 1.hf)) {
    return f16vec4(0.0hf);
  }

  return IPSample(tex, IPVec2Tile(coords, x_tile_mode, y_tile_mode), y_coord_scale);
}

/// Sample a texture, emulating a specific tile mode.
///
/// This is useful for Impeller graphics backend that don't have native support
/// for Decal.
f16vec4 IPSampleWithTileMode(sampler2D tex,
                          f16vec2 coords,
                          float16_t y_coord_scale,
                          float16_t tile_mode) {
  return IPSampleWithTileMode(tex, coords, y_coord_scale, tile_mode, tile_mode);
}

/// Sample a texture, emulating a specific tile mode.
///
/// This is useful for Impeller graphics backend that don't have native support
/// for Decal.
/// The range of `coods` will be mapped from [0, 1] to [half_texel, 1 - half_texel]
f16vec4 IPSampleLinearWithTileMode(sampler2D tex,
                                f16vec2 coords,
                                float16_t y_coord_scale,
                                f16vec2 half_texel,
                                float16_t x_tile_mode,
                                float16_t y_tile_mode) {
  if (x_tile_mode == float16_t(kTileModeDecal) && (coords.x < 0.0hf || coords.x >= 1.0hf) ||
      y_tile_mode == float16_t(kTileModeDecal) && (coords.y < 0.0hf || coords.y >= 1.0hf)) {
    return f16vec4(0.0hf);
  }

  return IPSampleLinear(tex, IPVec2Tile(coords, x_tile_mode, y_tile_mode), y_coord_scale, half_texel);
}

/// Sample a texture, emulating a specific tile mode.
///
/// This is useful for Impeller graphics backend that don't have native support
/// for Decal.
/// The range of `coods` will be mapped from [0, 1] to [half_texel, 1 - half_texel]
f16vec4 IPSampleLinearWithTileMode(sampler2D tex,
                                   f16vec2 coords,
                                   float16_t y_coord_scale,
                                   f16vec2 half_texel,
                                   float16_t tile_mode) {
  return IPSampleLinearWithTileMode(tex, coords, y_coord_scale, half_texel, tile_mode, tile_mode);
}

#endif
