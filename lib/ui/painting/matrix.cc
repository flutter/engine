// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/matrix.h"

#include "flutter/fml/logging.h"

namespace flutter {

// Mappings from SkMatrix-index to input-index.
static const int kSkMatrixIndexToMatrix4Index[] = {
    // clang-format off
    0, 4, 12,
    1, 5, 13,
    3, 7, 15,
    // clang-format on
};

SkMatrix ToSkMatrix(const tonic::Float64List& matrix4) {
  FML_DCHECK(matrix4.data());
  SkMatrix sk_matrix;
  for (int i = 0; i < 9; ++i) {
    int matrix4_index = kSkMatrixIndexToMatrix4Index[i];
    if (matrix4_index < matrix4.num_elements()) {
      sk_matrix[i] = matrix4[matrix4_index];
    } else {
      sk_matrix[i] = 0.0;
    }
  }
  return sk_matrix;
}

SkM44 ToSkM44(const tonic::Float64List& matrix4) {
  FML_DCHECK(matrix4.data());
  SkM44 sk_m44;
  for (int i = 0; i < 16; i += 1) {
    // row = i / 4 = i >> 2; col = i % 4 = i & 3;
    sk_m44.setRC(i >> 2, i & 3, i >= matrix4.num_elements() ? 0 : matrix4[i]);
  }
  return sk_m44;
}

tonic::Float64List ToMatrix4(const SkMatrix& sk_matrix) {
  tonic::Float64List matrix4(Dart_NewTypedData(Dart_TypedData_kFloat64, 16));
  for (int i = 0; i < 9; ++i) {
    matrix4[kSkMatrixIndexToMatrix4Index[i]] = sk_matrix[i];
  }
  matrix4[10] = 1.0;  // Identity along the z axis.
  return matrix4;
}

}  // namespace flutter
