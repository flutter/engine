// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_color_filter.h"
#include "flutter/display_list/testing/dl_test_equality.h"

namespace flutter {
namespace testing {

static const float kMatrix[20] = {
    1,  2,  3,  4,  5,   //
    6,  7,  8,  9,  10,  //
    11, 12, 13, 14, 15,  //
    16, 17, 18, 19, 20,  //
};

TEST(DisplayListColorFilter, BlendFactory) {
  auto cf = DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kDstATop);
  ASSERT_NE(cf.get(), nullptr);
  // ASSERT_EQ(cf->total_ref_count(), 1u);
  // ASSERT_EQ(cf->strong_ref_count(), 1u);
}

TEST(DisplayListColorFilter, BlendAsBlend) {
  auto cf = DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kDstATop);
  ASSERT_NE(cf->asBlend(), nullptr);
  ASSERT_EQ(cf->asBlend(), cf.get());
}

TEST(DisplayListColorFilter, BlendContents) {
  auto cf = DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kDstATop);
  ASSERT_EQ(cf->color(), DlColor::kRed());
  ASSERT_EQ(cf->mode(), DlBlendMode::kDstATop);
}

TEST(DisplayListColorFilter, BlendEquals) {
  auto cf1 = DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kDstATop);
  auto cf2 = DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kDstATop);
  TestEquals(*cf1, *cf2);
}

TEST(DisplayListColorFilter, BlendNotEquals) {
  auto cf1 = DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kDstATop);
  auto cf2 = DlBlendColorFilter::Make(DlColor::kBlue(), DlBlendMode::kDstATop);
  auto cf3 = DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kXor);
  TestNotEquals(*cf1, *cf2, "Color differs");
  TestNotEquals(*cf1, *cf3, "Blend mode differs");
}

TEST(DisplayListColorFilter, NopBlendProducesNull) {
  auto cf =
      DlBlendColorFilter::Make(DlColor::kTransparent(), DlBlendMode::kSrcOver);
  ASSERT_EQ(cf.get(), nullptr);
}

TEST(DisplayListColorFilter, MatrixFactory) {
  auto cf = DlMatrixColorFilter::Make(kMatrix);
  ASSERT_NE(cf.get(), nullptr);
  // ASSERT_EQ(cf->total_ref_count(), 1u);
  // ASSERT_EQ(cf->strong_ref_count(), 1u);
}

TEST(DisplayListColorFilter, MatrixAsMatrix) {
  auto cf = DlMatrixColorFilter::Make(kMatrix);
  ASSERT_NE(cf->asMatrix(), nullptr);
  ASSERT_EQ(cf->asMatrix(), cf.get());
}

TEST(DisplayListColorFilter, MatrixContents) {
  float matrix[20];
  memcpy(matrix, kMatrix, sizeof(matrix));
  auto cf = DlMatrixColorFilter::Make(matrix);

  // Test deref operator []
  for (int i = 0; i < 20; i++) {
    ASSERT_EQ((*cf)[i], matrix[i]);
  }

  // Test get_matrix
  float matrix2[20];
  cf->get_matrix(matrix2);
  for (int i = 0; i < 20; i++) {
    ASSERT_EQ(matrix2[i], matrix[i]);
  }

  // Test perturbing original array does not affect filter
  float original_value = matrix[4];
  matrix[4] += 101;
  ASSERT_EQ((*cf)[4], original_value);
}

TEST(DisplayListColorFilter, MatrixEquals) {
  auto cf1 = DlMatrixColorFilter::Make(kMatrix);
  auto cf2 = DlMatrixColorFilter::Make(kMatrix);
  TestEquals(*cf1, *cf2);
}

TEST(DisplayListColorFilter, MatrixNotEquals) {
  float matrix[20];
  memcpy(matrix, kMatrix, sizeof(matrix));
  auto cf1 = DlMatrixColorFilter::Make(matrix);
  matrix[4] += 101;
  auto cf2 = DlMatrixColorFilter::Make(matrix);
  TestNotEquals(*cf1, *cf2, "Matrix differs");
}

TEST(DisplayListColorFilter, NopMatrixProducesNull) {
  float matrix[20] = {
      1, 0, 0, 0, 0,  //
      0, 1, 0, 0, 0,  //
      0, 0, 1, 0, 0,  //
      0, 0, 0, 1, 0,  //
  };
  auto cf = DlMatrixColorFilter::Make(matrix);
  ASSERT_EQ(cf.get(), nullptr);
}

TEST(DisplayListColorFilter, SrgbToLinearFactory) {
  auto cf = DlSrgbToLinearGammaColorFilter::Make();
  ASSERT_NE(cf.get(), nullptr);
  // ASSERT_EQ(cf->total_ref_count(), 2u);
  // ASSERT_EQ(cf->strong_ref_count(), 2u);
}

TEST(DisplayListColorFilter, SrgbToLinearEquals) {
  auto cf1 = DlSrgbToLinearGammaColorFilter::Make();
  auto cf2 = DlSrgbToLinearGammaColorFilter::Make();
  TestEquals(*cf1, *cf2);
}

TEST(DisplayListColorFilter, LinearToSrgbFactory) {
  auto cf = DlLinearToSrgbGammaColorFilter::Make();
  ASSERT_NE(cf.get(), nullptr);
  // ASSERT_EQ(cf->total_ref_count(), 2u);
  // ASSERT_EQ(cf->strong_ref_count(), 2u);
}

TEST(DisplayListColorFilter, LinearToSrgbEquals) {
  auto cf1 = DlLinearToSrgbGammaColorFilter::Make();
  auto cf2 = DlLinearToSrgbGammaColorFilter::Make();
  TestEquals(*cf1, *cf2);
}

}  // namespace testing
}  // namespace flutter
