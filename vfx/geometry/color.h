// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GEOMETRY_COLOR_H_
#define VFX_GEOMETRY_COLOR_H_

#include <memory>

#include "vfx/geometry/offset.h"

namespace vfx {

// An RGBA color.
class Color {
 public:
  Color() { memset(data_, 0, sizeof(data_)); }
  explicit Color(float data[4]) { memcpy(data_, data, sizeof(data_)); }
  Color(float r, float g, float b, float a) {
    data_[0] = r;
    data_[1] = g;
    data_[2] = b;
    data_[3] = a;
  }

  float r() const { return data_[0]; }
  float g() const { return data_[1]; }
  float b() const { return data_[2]; }
  float a() const { return data_[3]; }

  const float* data() const { return data_; }

  void set_r(float r) { data_[0] = r; }
  void set_g(float g) { data_[1] = g; }
  void set_b(float b) { data_[2] = b; }
  void set_a(float a) { data_[3] = a; }

 private:
  float data_[4];
};

}  // namespace vfx

#endif  // VFX_GEOMETRY_COLOR_H_
