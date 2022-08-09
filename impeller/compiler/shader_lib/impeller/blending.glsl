// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BLENDING_GLSL_
#define BLENDING_GLSL_

#include <impeller/branching.glsl>
#include <impeller/constants.glsl>

//------------------------------------------------------------------------------
/// HSV utilities.
///

float IPLuminosity(vec3 color) {
  return color.r * 0.3 + color.g * 0.59 + color.b * 0.11;
}

/// Scales the color's luma by the amount necessary to place the color
/// components in a 1-0 range.
vec3 IPClipColor(vec3 color) {
  float lum = IPLuminosity(color);
  float mn = min(min(color.r, color.g), color.b);
  float mx = max(max(color.r, color.g), color.b);
  // `lum - mn` and `mx - lum` will always be >= 0 in the following conditions,
  // so adding a tiny value is enough to make these divisions safe.
  if (mn < 0.0) {
    color = lum + (((color - lum) * lum) / (lum - mn + kEhCloseEnough));
  }
  if (mx > 1.0) {
    color = lum + (((color - lum) * (1.0 - lum)) / (mx - lum + kEhCloseEnough));
  }
  return color;
}

vec3 IPSetLuminosity(vec3 color, float luminosity) {
  float relative_lum = luminosity - IPLuminosity(color);
  return IPClipColor(color + relative_lum);
}

float IPSaturation(vec3 color) {
  return max(max(color.r, color.g), color.b) -
         min(min(color.r, color.g), color.b);
}

vec3 IPSetSaturation(vec3 color, float saturation) {
  float mn = min(min(color.r, color.g), color.b);
  float mx = max(max(color.r, color.g), color.b);
  return (mn < mx) ? ((color - mn) * saturation) / (mx - mn) : vec3(0);
}

//------------------------------------------------------------------------------
/// Color blend functions.
///
/// These routines take two unpremultiplied RGB colors and output a third color.
/// They can be combined with any alpha compositing operation. When these blend
/// functions are used for drawing Entities in Impeller, the output is always
/// applied to the destination using `SourceOver` alpha compositing.
///

vec3 IPBlendScreen(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingscreen
  return dst + src - (dst * src);
}

vec3 IPBlendHardLight(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendinghardlight
  return IPVec3Choose(dst * (2.0 * src), IPBlendScreen(dst, 2.0 * src - 1.0),
                      src);
}

vec3 IPBlendOverlay(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingoverlay
  // HardLight, but with reversed parameters.
  return IPBlendHardLight(src, dst);
}

vec3 IPBlendDarken(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingdarken
  return min(dst, src);
}

vec3 IPBlendLighten(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendinglighten
  return max(dst, src);
}

vec3 IPBlendColorDodge(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingcolordodge

  vec3 color = min(vec3(1.0), dst / (1.0 - src));

  if (dst.r < kEhCloseEnough) {
    color.r = 0.0;
  }
  if (dst.g < kEhCloseEnough) {
    color.g = 0.0;
  }
  if (dst.b < kEhCloseEnough) {
    color.b = 0.0;
  }

  if (1.0 - src.r < kEhCloseEnough) {
    color.r = 1.0;
  }
  if (1.0 - src.g < kEhCloseEnough) {
    color.g = 1.0;
  }
  if (1.0 - src.b < kEhCloseEnough) {
    color.b = 1.0;
  }

  return color;
}

vec3 IPBlendColorBurn(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingcolorburn

  vec3 color = 1.0 - min(vec3(1.0), (1.0 - dst) / src);

  if (1.0 - dst.r < kEhCloseEnough) {
    color.r = 1.0;
  }
  if (1.0 - dst.g < kEhCloseEnough) {
    color.g = 1.0;
  }
  if (1.0 - dst.b < kEhCloseEnough) {
    color.b = 1.0;
  }

  if (src.r < kEhCloseEnough) {
    color.r = 0.0;
  }
  if (src.g < kEhCloseEnough) {
    color.g = 0.0;
  }
  if (src.b < kEhCloseEnough) {
    color.b = 0.0;
  }

  return color;
}

vec3 IPBlendSoftLight(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingsoftlight

  vec3 D = IPVec3ChooseCutoff(((16.0 * dst - 12.0) * dst + 4.0) * dst,  //
                              sqrt(dst),                                //
                              dst,                                      //
                              0.25);

  return IPVec3Choose(dst - (1.0 - 2.0 * src) * dst * (1.0 - dst),  //
                      dst + (2.0 * src - 1.0) * (D - dst),          //
                      src);
}

vec3 IPBlendDifference(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingdifference
  return abs(dst - src);
}

vec3 IPBlendExclusion(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingexclusion
  return dst + src - 2.0 * dst * src;
}

vec3 IPBlendMultiply(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingmultiply
  return dst * src;
}

vec3 IPBlendHue(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendinghue
  return IPSetLuminosity(IPSetSaturation(src, IPSaturation(dst)),
                         IPLuminosity(dst));
}

vec3 IPBlendSaturation(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingsaturation
  return IPSetLuminosity(IPSetSaturation(dst, IPSaturation(src)),
                         IPLuminosity(dst));
}

vec3 IPBlendColor(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingcolor
  return IPSetLuminosity(src, IPLuminosity(dst));
}

vec3 IPBlendLuminosity(vec3 dst, vec3 src) {
  // https://www.w3.org/TR/compositing-1/#blendingluminosity
  return IPSetLuminosity(dst, IPLuminosity(src));
}

#endif
