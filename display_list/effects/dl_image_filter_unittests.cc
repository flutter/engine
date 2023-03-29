// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/dl_blend_mode.h"
#include "flutter/display_list/dl_color.h"
#include "flutter/display_list/dl_sampling_options.h"
#include "flutter/display_list/dl_tile_mode.h"
#include "flutter/display_list/effects/dl_color_filter.h"
#include "flutter/display_list/effects/dl_image_filter.h"
#include "flutter/display_list/testing/dl_test_equality.h"
#include "flutter/display_list/utils/dl_comparable.h"
#include "gtest/gtest.h"

#include "third_party/skia/include/effects/SkImageFilters.h"

namespace flutter {
namespace testing {

// SkRect::contains treats the rect as a half-open interval which is
// appropriate for so many operations. Unfortunately, we are using
// it here to test containment of the corners of a transformed quad
// so the corners of the quad that are measured against the right
// and bottom edges are contained even if they are on the right or
// bottom edge. This method does the "all sides inclusive" version
// of SkRect::contains.
static bool containsInclusive(const SkRect rect, const SkPoint p) {
  // Test with a slight offset of 1E-9 to "forgive" IEEE bit-rounding
  // Ending up with bounds that are off by 1E-9 (these numbers are all
  // being tested in device space with this method) will be off by a
  // negligible amount of a pixel that wouldn't contribute to changing
  // the color of a pixel.
  return (p.fX >= rect.fLeft - 1E-9 &&   //
          p.fX <= rect.fRight + 1E-9 &&  //
          p.fY >= rect.fTop - 1E-9 &&    //
          p.fY <= rect.fBottom + 1E-9);
}

static bool containsInclusive(const SkRect rect, const SkPoint quad[4]) {
  return (containsInclusive(rect, quad[0]) &&  //
          containsInclusive(rect, quad[1]) &&  //
          containsInclusive(rect, quad[2]) &&  //
          containsInclusive(rect, quad[3]));
}

static bool containsInclusive(const SkIRect rect, const SkPoint quad[4]) {
  return containsInclusive(SkRect::Make(rect), quad);
}

static bool containsInclusive(const SkIRect rect, const SkRect bounds) {
  return (bounds.fLeft >= rect.fLeft - 1E-9 &&
          bounds.fTop >= rect.fTop - 1E-9 &&
          bounds.fRight <= rect.fRight + 1E-9 &&
          bounds.fBottom <= rect.fBottom + 1E-9);
}

// Used to verify that the expected output bounds and reverse-engineered
// "input bounds for output bounds" rectangles are included in the rectangle
// returned from the various bounds computation methods under the specified
// matrix.
static void TestBoundsWithMatrix(const DlImageFilter& filter,
                                 const SkMatrix& matrix,
                                 const SkRect& sourceBounds,
                                 const SkPoint expectedLocalOutputQuad[4]) {
  SkRect device_input_bounds = matrix.mapRect(sourceBounds);
  SkPoint expected_output_quad[4];
  matrix.mapPoints(expected_output_quad, expectedLocalOutputQuad, 4);

  SkIRect device_filter_ibounds;
  ASSERT_EQ(filter.map_device_bounds(device_input_bounds.roundOut(), matrix,
                                     device_filter_ibounds),
            &device_filter_ibounds);
  ASSERT_TRUE(containsInclusive(device_filter_ibounds, expected_output_quad));

  SkIRect reverse_input_ibounds;
  ASSERT_EQ(filter.get_input_device_bounds(device_filter_ibounds, matrix,
                                           reverse_input_ibounds),
            &reverse_input_ibounds);
  ASSERT_TRUE(containsInclusive(reverse_input_ibounds, device_input_bounds));
}

static void TestInvalidBounds(const DlImageFilter& filter,
                              const SkMatrix& matrix,
                              const SkRect& localInputBounds) {
  SkIRect device_input_bounds = matrix.mapRect(localInputBounds).roundOut();

  SkRect local_filter_bounds;
  ASSERT_EQ(filter.map_local_bounds(localInputBounds, local_filter_bounds),
            nullptr);
  ASSERT_EQ(local_filter_bounds, localInputBounds);

  SkIRect device_filter_ibounds;
  ASSERT_EQ(filter.map_device_bounds(device_input_bounds, matrix,
                                     device_filter_ibounds),
            nullptr);
  ASSERT_EQ(device_filter_ibounds, device_input_bounds);

  SkIRect reverse_input_ibounds;
  ASSERT_EQ(filter.get_input_device_bounds(device_input_bounds, matrix,
                                           reverse_input_ibounds),
            nullptr);
  ASSERT_EQ(reverse_input_ibounds, device_input_bounds);
}

// localInputBounds is a sample bounds for testing as input to the filter.
// localExpectOutputBounds is the theoretical output bounds for applying
// the filter to the localInputBounds.
// localExpectInputBounds is the theoretical input bounds required for the
// filter to cover the localExpectOutputBounds
// If either of the expected bounds are nullptr then the bounds methods will
// be assumed to be unable to perform their computations for the given
// image filter and will be returning null.
static void TestBounds(const DlImageFilter& filter,
                       const SkRect& sourceBounds,
                       const SkPoint expectedLocalOutputQuad[4]) {
  SkRect local_filter_bounds;
  ASSERT_EQ(filter.map_local_bounds(sourceBounds, local_filter_bounds),
            &local_filter_bounds);
  ASSERT_TRUE(containsInclusive(local_filter_bounds, expectedLocalOutputQuad));

  for (int scale = 1; scale <= 4; scale++) {
    for (int skew = 0; skew < 8; skew++) {
      for (int degrees = 0; degrees <= 360; degrees += 15) {
        SkMatrix matrix;
        matrix.setScale(scale, scale);
        matrix.postSkew(skew / 8.0, skew / 8.0);
        matrix.postRotate(degrees);
        ASSERT_TRUE(matrix.invert(nullptr));
        TestBoundsWithMatrix(filter, matrix, sourceBounds,
                             expectedLocalOutputQuad);
        matrix.setPerspX(0.001);
        matrix.setPerspY(0.001);
        ASSERT_TRUE(matrix.invert(nullptr));
        TestBoundsWithMatrix(filter, matrix, sourceBounds,
                             expectedLocalOutputQuad);
      }
    }
  }
}

static void TestBounds(const DlImageFilter& filter,
                       const SkRect& sourceBounds,
                       const SkRect& expectedLocalOutputBounds) {
  SkPoint expected_local_output_quad[4];
  expectedLocalOutputBounds.toQuad(expected_local_output_quad);
  TestBounds(filter, sourceBounds, expected_local_output_quad);
}

TEST(DisplayListImageFilter, BlurConstructor) {
  auto filter = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
}

TEST(DisplayListImageFilter, BlurAsBlur) {
  auto filter = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);

  ASSERT_NE(filter->asBlur(), nullptr);
  ASSERT_EQ(filter->asBlur(), filter.get());
}

TEST(DisplayListImageFilter, BlurContents) {
  auto filter = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);

  ASSERT_EQ(filter->sigma_x(), 5.0);
  ASSERT_EQ(filter->sigma_y(), 6.0);
  ASSERT_EQ(filter->tile_mode(), DlTileMode::kMirror);
}

TEST(DisplayListImageFilter, BlurEquals) {
  auto filter1 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter2 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);

  TestEquals(*filter1, *filter2);
}

TEST(DisplayListImageFilter, BlurWithLocalMatrixEquals) {
  auto filter1 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter2 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);

  SkMatrix local_matrix = SkMatrix::Translate(10, 10);
  TestEquals(*filter1->makeWithLocalMatrix(local_matrix),
             *filter2->makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, BlurNotEquals) {
  auto filter1 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter2 = DlBlurImageFilter::Make(7.0, 6.0, DlTileMode::kMirror);
  auto filter3 = DlBlurImageFilter::Make(5.0, 8.0, DlTileMode::kMirror);
  auto filter4 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kRepeat);

  TestNotEquals(*filter1, *filter2, "Sigma X differs");
  TestNotEquals(*filter1, *filter3, "Sigma Y differs");
  TestNotEquals(*filter1, *filter4, "Tile Mode differs");
}

TEST(DisplayListImageFilter, BlurBounds) {
  auto filter = DlBlurImageFilter::Make(5, 10, DlTileMode::kDecal);
  SkRect input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  SkRect expected_output_bounds = input_bounds.makeOutset(15, 30);
  TestBounds(*filter, input_bounds, expected_output_bounds);
}

TEST(DisplayListImageFilter, DilateConstructor) {
  auto filter = DlDilateImageFilter::Make(5.0, 6.0);
}

TEST(DisplayListImageFilter, DilateAsDilate) {
  auto filter = DlDilateImageFilter::Make(5.0, 6.0);

  ASSERT_NE(filter->asDilate(), nullptr);
  ASSERT_EQ(filter->asDilate(), filter.get());
}

TEST(DisplayListImageFilter, DilateContents) {
  auto filter = DlDilateImageFilter::Make(5.0, 6.0);

  ASSERT_EQ(filter->radius_x(), 5.0);
  ASSERT_EQ(filter->radius_y(), 6.0);
}

TEST(DisplayListImageFilter, DilateEquals) {
  auto filter1 = DlDilateImageFilter::Make(5.0, 6.0);
  auto filter2 = DlDilateImageFilter::Make(5.0, 6.0);

  TestEquals(*filter1, *filter2);
}

TEST(DisplayListImageFilter, DilateWithLocalMatrixEquals) {
  auto filter1 = DlDilateImageFilter::Make(5.0, 6.0);
  auto filter2 = DlDilateImageFilter::Make(5.0, 6.0);

  SkMatrix local_matrix = SkMatrix::Translate(10, 10);
  TestEquals(*filter1->makeWithLocalMatrix(local_matrix),
             *filter2->makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, DilateNotEquals) {
  auto filter1 = DlDilateImageFilter::Make(5.0, 6.0);
  auto filter2 = DlDilateImageFilter::Make(7.0, 6.0);
  auto filter3 = DlDilateImageFilter::Make(5.0, 8.0);

  TestNotEquals(*filter1, *filter2, "Radius X differs");
  TestNotEquals(*filter1, *filter3, "Radius Y differs");
}

TEST(DisplayListImageFilter, DilateBounds) {
  auto filter = DlDilateImageFilter::Make(5, 10);
  SkRect input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  SkRect expected_output_bounds = input_bounds.makeOutset(5, 10);
  TestBounds(*filter, input_bounds, expected_output_bounds);
}

TEST(DisplayListImageFilter, ErodeConstructor) {
  auto filter = DlErodeImageFilter::Make(5.0, 6.0);
}

TEST(DisplayListImageFilter, ErodeAsErode) {
  auto filter = DlErodeImageFilter::Make(5.0, 6.0);

  ASSERT_NE(filter->asErode(), nullptr);
  ASSERT_EQ(filter->asErode(), filter.get());
}

TEST(DisplayListImageFilter, ErodeContents) {
  auto filter = DlErodeImageFilter::Make(5.0, 6.0);

  ASSERT_EQ(filter->radius_x(), 5.0);
  ASSERT_EQ(filter->radius_y(), 6.0);
}

TEST(DisplayListImageFilter, ErodeEquals) {
  auto filter1 = DlErodeImageFilter::Make(5.0, 6.0);
  auto filter2 = DlErodeImageFilter::Make(5.0, 6.0);

  TestEquals(*filter1, *filter2);
}

TEST(DisplayListImageFilter, ErodeWithLocalMatrixEquals) {
  auto filter1 = DlErodeImageFilter::Make(5.0, 6.0);
  auto filter2 = DlErodeImageFilter::Make(5.0, 6.0);

  SkMatrix local_matrix = SkMatrix::Translate(10, 10);
  TestEquals(*filter1->makeWithLocalMatrix(local_matrix),
             *filter2->makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, ErodeNotEquals) {
  auto filter1 = DlErodeImageFilter::Make(5.0, 6.0);
  auto filter2 = DlErodeImageFilter::Make(7.0, 6.0);
  auto filter3 = DlErodeImageFilter::Make(5.0, 8.0);

  TestNotEquals(*filter1, *filter2, "Radius X differs");
  TestNotEquals(*filter1, *filter3, "Radius Y differs");
}

TEST(DisplayListImageFilter, ErodeBounds) {
  auto filter = DlErodeImageFilter::Make(5, 10);
  SkRect input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  SkRect expected_output_bounds = input_bounds.makeInset(5, 10);
  TestBounds(*filter, input_bounds, expected_output_bounds);
}

TEST(DisplayListImageFilter, MatrixConstructor) {
  auto filter = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                            0.5, 3.0, 15,  //
                                                            0.0, 0.0, 1),
                                          DlImageSampling::kLinear);
}

TEST(DisplayListImageFilter, MatrixAsMatrix) {
  auto filter = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                            0.5, 3.0, 15,  //
                                                            0.0, 0.0, 1),
                                          DlImageSampling::kLinear);

  ASSERT_NE(filter->asMatrix(), nullptr);
  ASSERT_EQ(filter->asMatrix(), filter.get());
}

TEST(DisplayListImageFilter, MatrixContents) {
  SkMatrix matrix = SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                      0.5, 3.0, 15,  //
                                      0.0, 0.0, 1);
  auto filter = DlMatrixImageFilter::Make(matrix, DlImageSampling::kLinear);

  ASSERT_EQ(filter->matrix(), matrix);
  ASSERT_EQ(filter->sampling(), DlImageSampling::kLinear);
}

TEST(DisplayListImageFilter, MatrixEquals) {
  SkMatrix matrix = SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                      0.5, 3.0, 15,  //
                                      0.0, 0.0, 1);
  auto filter1 = DlMatrixImageFilter::Make(matrix, DlImageSampling::kLinear);
  auto filter2 = DlMatrixImageFilter::Make(matrix, DlImageSampling::kLinear);

  TestEquals(*filter1, *filter2);
}

TEST(DisplayListImageFilter, MatrixWithLocalMatrixEquals) {
  SkMatrix matrix = SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                      0.5, 3.0, 15,  //
                                      0.0, 0.0, 1);
  auto filter1 = DlMatrixImageFilter::Make(matrix, DlImageSampling::kLinear);
  auto filter2 = DlMatrixImageFilter::Make(matrix, DlImageSampling::kLinear);

  SkMatrix local_matrix = SkMatrix::Translate(10, 10);
  TestEquals(*filter1->makeWithLocalMatrix(local_matrix),
             *filter2->makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, MatrixNotEquals) {
  SkMatrix matrix1 = SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                       0.5, 3.0, 15,  //
                                       0.0, 0.0, 1);
  SkMatrix matrix2 = SkMatrix::MakeAll(5.0, 0.0, 10,  //
                                       0.5, 3.0, 15,  //
                                       0.0, 0.0, 1);
  auto filter1 = DlMatrixImageFilter::Make(matrix1, DlImageSampling::kLinear);
  auto filter2 = DlMatrixImageFilter::Make(matrix2, DlImageSampling::kLinear);
  auto filter3 =
      DlMatrixImageFilter::Make(matrix1, DlImageSampling::kNearestNeighbor);

  TestNotEquals(*filter1, *filter2, "Matrix differs");
  TestNotEquals(*filter1, *filter3, "Sampling differs");
}

TEST(DisplayListImageFilter, MatrixBounds) {
  SkMatrix matrix = SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                      0.5, 3.0, 7,   //
                                      0.0, 0.0, 1);
  SkMatrix inverse;
  ASSERT_TRUE(matrix.invert(&inverse));
  auto filter = DlMatrixImageFilter::Make(matrix, DlImageSampling::kLinear);
  SkRect input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  SkPoint expectedOutputQuad[4] = {
      {50, 77},    // (20,20) => (20*2 + 10, 20/2 + 20*3 + 7) == (50, 77)
      {50, 257},   // (20,80) => (20*2 + 10, 20/2 + 80*3 + 7) == (50, 257)
      {170, 287},  // (80,80) => (80*2 + 10, 80/2 + 80*3 + 7) == (170, 287)
      {170, 107},  // (80,20) => (80*2 + 10, 80/2 + 20*3 + 7) == (170, 107)
  };
  TestBounds(*filter, input_bounds, expectedOutputQuad);
}

TEST(DisplayListImageFilter, ComposeConstructor) {
  auto outer = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                           0.5, 3.0, 15,  //
                                                           0.0, 0.0, 1),
                                         DlImageSampling::kLinear);
  auto inner = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter = DlComposeImageFilter::Make(outer, inner);
}

TEST(DisplayListImageFilter, ComposeAsCompose) {
  auto outer = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                           0.5, 3.0, 15,  //
                                                           0.0, 0.0, 1),
                                         DlImageSampling::kLinear);
  auto inner = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter = DlComposeImageFilter::Make(outer, inner);

  ASSERT_NE(filter->asCompose(), nullptr);
  ASSERT_EQ(filter->asCompose(), filter.get());
}

TEST(DisplayListImageFilter, ComposeContents) {
  auto outer = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                           0.5, 3.0, 15,  //
                                                           0.0, 0.0, 1),
                                         DlImageSampling::kLinear);
  auto inner = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter = DlComposeImageFilter::Make(outer, inner);
  const DlComposeImageFilter* compose = filter->asCompose();
  ASSERT_NE(compose, nullptr);

  ASSERT_EQ(compose->outer(), outer);
  ASSERT_EQ(compose->inner(), inner);
}

TEST(DisplayListImageFilter, ComposeEquals) {
  auto outer1 = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                            0.5, 3.0, 15,  //
                                                            0.0, 0.0, 1),
                                          DlImageSampling::kLinear);
  auto inner1 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter1 = DlComposeImageFilter::Make(outer1, inner1);

  auto outer2 = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                            0.5, 3.0, 15,  //
                                                            0.0, 0.0, 1),
                                          DlImageSampling::kLinear);
  auto inner2 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter2 = DlComposeImageFilter::Make(outer1, inner1);

  TestEquals(*filter1, *filter2);
}

TEST(DisplayListImageFilter, ComposeWithLocalMatrixEquals) {
  auto outer1 = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                            0.5, 3.0, 15,  //
                                                            0.0, 0.0, 1),
                                          DlImageSampling::kLinear);
  auto inner1 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter1 = DlComposeImageFilter::Make(outer1, inner1);

  auto outer2 = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                            0.5, 3.0, 15,  //
                                                            0.0, 0.0, 1),
                                          DlImageSampling::kLinear);
  auto inner2 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);
  auto filter2 = DlComposeImageFilter::Make(outer1, inner1);

  SkMatrix local_matrix = SkMatrix::Translate(10, 10);
  TestEquals(*filter1->makeWithLocalMatrix(local_matrix),
             *filter2->makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, ComposeNotEquals) {
  auto outer1 = DlMatrixImageFilter::Make(SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                                            0.5, 3.0, 15,  //
                                                            0.0, 0.0, 1),
                                          DlImageSampling::kLinear);
  auto inner1 = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kMirror);

  auto outer2 = DlMatrixImageFilter::Make(SkMatrix::MakeAll(5.0, 0.0, 10,  //
                                                            0.5, 3.0, 15,  //
                                                            0.0, 0.0, 1),
                                          DlImageSampling::kLinear);
  auto inner2 = DlBlurImageFilter::Make(7.0, 6.0, DlTileMode::kMirror);

  auto filter1 = DlComposeImageFilter::Make(outer1, inner1);
  auto filter2 = DlComposeImageFilter::Make(outer2, inner1);
  auto filter3 = DlComposeImageFilter::Make(outer1, inner2);

  TestNotEquals(*filter1, *filter2, "Outer differs");
  TestNotEquals(*filter1, *filter3, "Inner differs");
}

TEST(DisplayListImageFilter, ComposeBounds) {
  auto outer = DlDilateImageFilter::Make(5, 10);
  auto inner = DlBlurImageFilter::Make(12, 5, DlTileMode::kDecal);
  auto filter = DlComposeImageFilter::Make(outer, inner);
  SkRect input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  SkRect expected_output_bounds =
      input_bounds.makeOutset(36, 15).makeOutset(5, 10);
  TestBounds(*filter, input_bounds, expected_output_bounds);
}

static void TestUnboundedBounds(const DlImageFilter& filter,
                                const SkRect& sourceBounds,
                                const SkRect& expectedOutputBounds,
                                const SkRect& expectedInputBounds) {
  SkRect bounds;
  EXPECT_EQ(filter.map_local_bounds(sourceBounds, bounds), nullptr);
  EXPECT_EQ(bounds, expectedOutputBounds);

  SkIRect ibounds;
  EXPECT_EQ(
      filter.map_device_bounds(sourceBounds.roundOut(), SkMatrix::I(), ibounds),
      nullptr);
  EXPECT_EQ(ibounds, expectedOutputBounds.roundOut());

  EXPECT_EQ(filter.get_input_device_bounds(sourceBounds.roundOut(),
                                           SkMatrix::I(), ibounds),
            nullptr);
  EXPECT_EQ(ibounds, expectedInputBounds.roundOut());
}

TEST(DisplayListImageFilter, ComposeBoundsWithUnboundedInner) {
  auto input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  auto expected_bounds = SkRect::MakeLTRB(5, 2, 95, 98);

  auto color_filter =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kSrcOver);
  auto outer = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kRepeat);
  auto inner = DlColorFilterImageFilter::Make(color_filter);
  auto composed = DlComposeImageFilter::Make(outer, inner);

  TestUnboundedBounds(*composed, input_bounds, expected_bounds,
                      expected_bounds);
}

TEST(DisplayListImageFilter, ComposeBoundsWithUnboundedOuter) {
  auto input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  auto expected_bounds = SkRect::MakeLTRB(5, 2, 95, 98);

  auto color_filter =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kSrcOver);
  auto outer = DlColorFilterImageFilter::Make(color_filter);
  auto inner = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kRepeat);
  auto composed = DlComposeImageFilter::Make(outer, inner);

  TestUnboundedBounds(*composed, input_bounds, expected_bounds,
                      expected_bounds);
}

TEST(DisplayListImageFilter, ComposeBoundsWithUnboundedInnerAndOuter) {
  auto input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  auto expected_bounds = input_bounds;

  auto color_filter1 =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kSrcOver);
  auto color_filter2 =
      DlBlendColorFilter::Make(DlColor::kBlue(), DlBlendMode::kSrcOver);
  auto outer = DlColorFilterImageFilter::Make(color_filter1);
  auto inner = DlColorFilterImageFilter::Make(color_filter2);
  auto composed = DlComposeImageFilter::Make(outer, inner);

  TestUnboundedBounds(*composed, input_bounds, expected_bounds,
                      expected_bounds);
}

// See https://github.com/flutter/flutter/issues/108433
TEST(DisplayListImageFilter, Issue108433) {
  auto input_bounds = SkIRect::MakeLTRB(20, 20, 80, 80);

  auto sk_filter = SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcOver);
  auto sk_outer = SkImageFilters::Blur(5.0, 6.0, SkTileMode::kRepeat, nullptr);
  auto sk_inner = SkImageFilters::ColorFilter(sk_filter, nullptr);
  auto sk_compose = SkImageFilters::Compose(sk_outer, sk_inner);

  auto dl_filter =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kSrcOver);
  auto dl_outer = DlBlurImageFilter::Make(5.0, 6.0, DlTileMode::kRepeat);
  auto dl_inner = DlColorFilterImageFilter::Make(dl_filter);
  auto dl_compose = DlComposeImageFilter::Make(dl_outer, dl_inner);

  auto sk_bounds = sk_compose->filterBounds(
      input_bounds, SkMatrix::I(),
      SkImageFilter::MapDirection::kForward_MapDirection);

  SkIRect dl_bounds;
  EXPECT_EQ(
      dl_compose->map_device_bounds(input_bounds, SkMatrix::I(), dl_bounds),
      nullptr);
  ASSERT_EQ(dl_bounds, sk_bounds);
}

TEST(DisplayListImageFilter, ColorFilterConstructor) {
  auto dl_color_filter =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kLighten);
  auto filter = DlColorFilterImageFilter::Make(dl_color_filter);
}

TEST(DisplayListImageFilter, ColorFilterAsColorFilter) {
  auto dl_color_filter =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kLighten);
  auto filter = DlColorFilterImageFilter::Make(dl_color_filter);

  ASSERT_NE(filter->asColorFilter(), nullptr);
  ASSERT_EQ(filter->asColorFilter(), filter.get());
}

TEST(DisplayListImageFilter, ColorFilterContents) {
  auto dl_color_filter =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kLighten);
  auto filter = DlColorFilterImageFilter::Make(dl_color_filter);

  ASSERT_TRUE(Equals(filter->color_filter(), dl_color_filter));
}

TEST(DisplayListImageFilter, ColorFilterEquals) {
  auto dl_color_filter1 =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kLighten);
  auto filter1 = DlColorFilterImageFilter::Make(dl_color_filter1);

  auto dl_color_filter2 =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kLighten);
  auto filter2 = DlColorFilterImageFilter::Make(dl_color_filter2);

  TestEquals(*filter1, *filter2);
}

TEST(DisplayListImageFilter, ColorFilterWithLocalMatrixEquals) {
  auto dl_color_filter1 =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kLighten);
  auto filter1 = DlColorFilterImageFilter::Make(dl_color_filter1);

  auto dl_color_filter2 =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kLighten);
  auto filter2 = DlColorFilterImageFilter::Make(dl_color_filter2);

  SkMatrix local_matrix = SkMatrix::Translate(10, 10);
  TestEquals(*filter1->makeWithLocalMatrix(local_matrix),
             *filter2->makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, ColorFilterNotEquals) {
  auto dl_color_filter1 =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kLighten);
  auto filter1 = DlColorFilterImageFilter::Make(dl_color_filter1);

  auto dl_color_filter2 =
      DlBlendColorFilter::Make(DlColor::kBlue(), DlBlendMode::kLighten);
  auto filter2 = DlColorFilterImageFilter::Make(dl_color_filter2);

  auto dl_color_filter3 =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kDarken);
  auto filter3 = DlColorFilterImageFilter::Make(dl_color_filter3);

  TestNotEquals(*filter1, *filter2, "Color differs");
  TestNotEquals(*filter1, *filter3, "Blend Mode differs");
}

TEST(DisplayListImageFilter, ColorFilterBounds) {
  auto dl_color_filter =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kSrcIn);
  auto filter = DlColorFilterImageFilter::Make(dl_color_filter);
  SkRect input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  TestBounds(*filter, input_bounds, input_bounds);
}

TEST(DisplayListImageFilter, ColorFilterModifiesTransparencyBounds) {
  auto dl_color_filter =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kSrcOver);
  auto filter = DlColorFilterImageFilter::Make(dl_color_filter);
  SkRect input_bounds = SkRect::MakeLTRB(20, 20, 80, 80);
  TestInvalidBounds(*filter, SkMatrix::I(), input_bounds);
}

TEST(DisplayListImageFilter, LocalImageFilterBounds) {
  auto filter_matrix = SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                         0.5, 3.0, 15,  //
                                         0.0, 0.0, 1);
  std::vector<sk_sp<SkImageFilter>> sk_filters{
      SkImageFilters::Blur(5.0, 6.0, SkTileMode::kRepeat, nullptr),
      SkImageFilters::ColorFilter(
          SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcOver), nullptr),
      SkImageFilters::Dilate(5.0, 10.0, nullptr),
      SkImageFilters::MatrixTransform(
          filter_matrix, SkSamplingOptions(SkFilterMode::kLinear), nullptr),
      SkImageFilters::Compose(
          SkImageFilters::Blur(5.0, 6.0, SkTileMode::kRepeat, nullptr),
          SkImageFilters::ColorFilter(
              SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcOver),
              nullptr))};

  auto dl_color_filter =
      DlBlendColorFilter::Make(DlColor::kRed(), DlBlendMode::kSrcOver);
  std::vector<dl_shared<const DlImageFilter>> dl_filters{
      DlImageFilter::MakeBlur(5.0, 6.0, DlTileMode::kRepeat),
      DlImageFilter::MakeColorFilter(dl_color_filter),
      DlImageFilter::MakeDilate(5, 10),
      DlImageFilter::MakeMatrix(filter_matrix, DlImageSampling::kLinear),
      DlImageFilter::MakeCompose(
          DlImageFilter::MakeBlur(5.0, 6.0, DlTileMode::kRepeat),
          DlImageFilter::MakeColorFilter(dl_color_filter))};

  auto persp = SkMatrix::I();
  persp.setPerspY(0.001);
  std::vector<SkMatrix> matrices = {
      SkMatrix::Translate(10.0, 10.0),
      SkMatrix::Scale(2.0, 2.0).preTranslate(10.0, 10.0),
      SkMatrix::RotateDeg(45).preTranslate(5.0, 5.0), persp};
  std::vector<SkMatrix> bounds_matrices{SkMatrix::Translate(5.0, 10.0),
                                        SkMatrix::Scale(2.0, 2.0)};

  for (unsigned i = 0; i < sk_filters.size(); i++) {
    for (unsigned j = 0; j < matrices.size(); j++) {
      for (unsigned k = 0; k < bounds_matrices.size(); k++) {
        auto& m = matrices[j];
        auto& bounds_matrix = bounds_matrices[k];
        auto sk_local_filter = sk_filters[i]->makeWithLocalMatrix(m);
        auto dl_local_filter = dl_filters[i]->makeWithLocalMatrix(m);
        if (!sk_local_filter || !dl_local_filter) {
          // Temporarily relax the equivalence testing to allow Skia to expand
          // their behavior. Once the Skia fixes are rolled in, the
          // DlImageFilter should adapt  to the new rules.
          // See https://github.com/flutter/flutter/issues/114723
          ASSERT_TRUE(sk_local_filter || !dl_local_filter);
          continue;
        }
        {
          auto input_bounds = SkIRect::MakeLTRB(20, 20, 80, 80);
          SkIRect sk_rect, dl_rect;
          sk_rect = sk_local_filter->filterBounds(
              input_bounds, bounds_matrix,
              SkImageFilter::MapDirection::kForward_MapDirection);
          dl_local_filter->map_device_bounds(input_bounds, bounds_matrix,
                                             dl_rect);
          ASSERT_EQ(sk_rect, dl_rect);
        }
        {
          // Test for: Know the outset bounds to get the inset bounds
          // Skia have some bounds calculate error of DilateFilter and
          // MatrixFilter
          // Skia issue: https://bugs.chromium.org/p/skia/issues/detail?id=13444
          // flutter issue: https://github.com/flutter/flutter/issues/108693
          if (i == 2 || i == 3) {
            continue;
          }
          auto outset_bounds = SkIRect::MakeLTRB(20, 20, 80, 80);
          SkIRect sk_rect, dl_rect;
          sk_rect = sk_local_filter->filterBounds(
              outset_bounds, bounds_matrix,
              SkImageFilter::MapDirection::kReverse_MapDirection);
          dl_local_filter->get_input_device_bounds(outset_bounds, bounds_matrix,
                                                   dl_rect);
          ASSERT_EQ(sk_rect, dl_rect);
        }
      }
    }
  }
}

}  // namespace testing
}  // namespace flutter
