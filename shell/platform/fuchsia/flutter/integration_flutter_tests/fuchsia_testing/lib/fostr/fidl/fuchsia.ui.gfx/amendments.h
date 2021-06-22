// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_FOSTR_FIDL_FUCHSIA_UI_GFX_AMENDMENTS_H_
#define LIB_FOSTR_FIDL_FUCHSIA_UI_GFX_AMENDMENTS_H_

#include <fuchsia/ui/gfx/cpp/fidl.h>

#include <iosfwd>

namespace fuchsia {
namespace ui {
namespace gfx {

// NOTE:
// //garnet/public/lib/fostr/fidl/fuchsia.ui.gfx generates ostream formatters
// for this library *except* those formatters that are listed here. The code
// generator knows which formatters to exclude from the generated code by
// consulting the 'amendments.json' file in that directory.
//
// If you add or remove formatters from this file, please be sure that the
// amendments.json file is updated accordingly.

// NOTE: fostr doesn't generate operator<< for union tags, so we explicitly add
// our own where desired.

std::ostream& operator<<(std::ostream& stream, const fuchsia::ui::gfx::Value::Tag& tag);

inline vec2 operator-(const vec2& v) { return {.x = -v.x, .y = -v.y}; }

inline vec2& operator+=(vec2& a, const vec2& b) {
  a.x += b.x;
  a.y += b.y;
  return a;
}

// Return a vec2 consisting of the component-wise sum of the two arguments.
inline vec2 operator+(const vec2& a, const vec2& b) { return {.x = a.x + b.x, .y = a.y + b.y}; }

// Return a vec2 consisting of the component-wise difference of the two args.
inline vec2 operator-(const vec2& a, const vec2& b) { return {.x = a.x - b.x, .y = a.y - b.y}; }

inline vec2 operator*(const vec2& v, float s) { return {.x = v.x * s, .y = v.y * s}; }

inline vec2& operator/=(vec2& v, float s) {
  v.x /= s;
  v.y /= s;
  return v;
}

inline vec2 operator/(const vec2& v, float s) { return {.x = v.x / s, .y = v.y / s}; }

// Return a vec3 consisting of the component-wise sum of the two arguments.
inline vec3 operator+(const vec3& a, const vec3& b) {
  return {.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

// Return a vec3 consisting of the component-wise difference of the two args.
inline vec3 operator-(const vec3& a, const vec3& b) {
  return {.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z};
}

// Return a vec4 consisting of the component-wise sum of the two arguments.
inline vec4 operator+(const vec4& a, const vec4& b) {
  return {.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z, .w = a.w + b.w};
}

// Return a vec4 consisting of the component-wise difference of the two args.
inline vec4 operator-(const vec4& a, const vec4& b) {
  return {.x = a.x - b.x, .y = a.y - b.y, .z = a.z - b.z, .w = a.w - b.w};
}

}  // namespace gfx
}  // namespace ui
}  // namespace fuchsia

#endif  // LIB_FOSTR_FIDL_FUCHSIA_UI_GFX_AMENDMENTS_H_
