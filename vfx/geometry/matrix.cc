// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/geometry/matrix.h"

#include <math.h>

namespace vfx {

static_assert(sizeof(Matrix) == sizeof(float) * 16,
              "Matrix should contain only floats");

void Matrix::SetZero() {
  memset(data_, 0, sizeof(data_));
}

void Matrix::SetIdentity() {
  memset(data_, 0, sizeof(data_));
  data_[0][0] = 1.0f;
  data_[1][1] = 1.0f;
  data_[2][2] = 1.0f;
  data_[3][3] = 1.0f;
}

void Matrix::SetFrustum(float left,
                        float right,
                        float bottom,
                        float top,
                        float near_z,
                        float far_z) {
  float dx = right - left;
  float dy = top - bottom;
  float dz = far_z - near_z;

  if ((near_z <= 0.0f) ||
      (far_z <= 0.0f) ||
      (dx <= 0.0f) ||
      (dy <= 0.0f) ||
      (dz <= 0.0f)) {
    SetIdentity();
  } else {
    data_[0][0] = 2.0f * near_z / dx;
    data_[1][0] = 0.0f;
    data_[2][0] = 0.0f;
    data_[3][0] = 0.0f;

    data_[0][1] = 0.0f;
    data_[1][1] = 2.0f * near_z / dy;
    data_[2][1] = 0.0f;
    data_[3][1] = 0.0f;

    data_[0][2] = (right + left) / dx;
    data_[1][2] = (top + bottom) / dy;
    data_[2][2] = -(near_z + far_z) / dz;
    data_[3][2] = -1.0f;

    data_[0][3] = 0.0f;
    data_[1][3] = 0.0f;
    data_[2][3] = -2.0f * near_z * far_z / dz;
    data_[3][3] = 0.0f;
  }
}

void Matrix::SetPerspective(float fov_y,
                            float aspect,
                            float near_z,
                            float far_z) {
  float dy = tanf(fov_y / 360.0f * M_PI) * near_z;
  float dx = dy * aspect;
  SetFrustum(-dx, dx, -dy, dy, near_z, far_z);
}

void Matrix::PreTranslate(const Offset& offset) {
  float dx = offset.dx();
  float dy = offset.dy();
  float dz = offset.dz();

  data_[3][0] += dx * data_[0][0] + dy * data_[1][0] + dz * data_[2][0];
  data_[3][1] += dx * data_[0][1] + dy * data_[1][1] + dz * data_[2][1];
  data_[3][2] += dx * data_[0][2] + dy * data_[1][2] + dz * data_[2][2];
  data_[3][3] += dx * data_[0][3] + dy * data_[1][3] + dz * data_[2][3];
}

void Matrix::PostTranslate(const Offset& offset) {
  float dx = offset.dx();
  if (dx != 0) {
    data_[0][0] += data_[0][3] * dx;
    data_[1][0] += data_[1][3] * dx;
    data_[2][0] += data_[2][3] * dx;
    data_[3][0] += data_[3][3] * dx;
  }

  float dy = offset.dy();
  if (dy != 0) {
    data_[0][1] += data_[0][3] * dy;
    data_[1][1] += data_[1][3] * dy;
    data_[2][1] += data_[2][3] * dy;
    data_[3][1] += data_[3][3] * dy;
  }

  float dz = offset.dz();
  if (dz != 0) {
    data_[0][2] += data_[0][3] * dz;
    data_[1][2] += data_[1][3] * dz;
    data_[2][2] += data_[2][3] * dz;
    data_[3][2] += data_[3][3] * dz;
  }
}

}  // namespace vfx
