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

#include "third_party/skia/include/core/SkBlendMode.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkSamplingOptions.h"
#include "third_party/skia/include/effects/SkImageFilters.h"

namespace flutter {
namespace testing {

// DlTRect::contains treats the rect as a half-open interval which is
// appropriate for so many operations. Unfortunately, we are using
// it here to test containment of the corners of a transformed quad
// so the corners of the quad that are measured against the right
// and bottom edges are contained even if they are on the right or
// bottom edge. This method does the "all sides inclusive" version
// of DlTRect::contains.
static bool containsInclusive(const DlFRect rect, const DlFPoint p) {
  // Test with a slight offset of 1E-9 to "forgive" IEEE bit-rounding
  // Ending up with bounds that are off by 1E-9 (these numbers are all
  // being tested in device space with this method) will be off by a
  // negligible amount of a pixel that wouldn't contribute to changing
  // the color of a pixel.
  return (p.x() >= rect.left() - 1E-9 &&   //
          p.x() <= rect.right() + 1E-9 &&  //
          p.y() >= rect.top() - 1E-9 &&    //
          p.y() <= rect.bottom() + 1E-9);
}

static bool containsInclusive(const DlFRect rect, const DlFPoint quad[4]) {
  return (containsInclusive(rect, quad[0]) &&  //
          containsInclusive(rect, quad[1]) &&  //
          containsInclusive(rect, quad[2]) &&  //
          containsInclusive(rect, quad[3]));
}

static bool containsInclusive(const DlIRect rect, const DlFPoint quad[4]) {
  return containsInclusive(DlFRect::MakeBounds(rect), quad);
}

static bool containsInclusive(const DlIRect rect, const DlFRect bounds) {
  return (bounds.left() >= rect.left() - 1E-9 &&
          bounds.top() >= rect.top() - 1E-9 &&
          bounds.right() <= rect.right() + 1E-9 &&
          bounds.bottom() <= rect.bottom() + 1E-9);
}

// Used to verify that the expected output bounds and reverse-engineered
// "input bounds for output bounds" rectangles are included in the rectangle
// returned from the various bounds computation methods under the specified
// matrix.
static void TestBoundsWithMatrix(const DlImageFilter& filter,
                                 const DlTransform& matrix,
                                 const DlFRect& sourceBounds,
                                 const DlFPoint expectedLocalOutputQuad[4]) {
  DlFRect device_input_bounds = matrix.TransformRect(sourceBounds);
  DlIRect device_input_ibounds = DlIRect::MakeRoundedOut(device_input_bounds);
  DlFPoint expected_output_quad[4];
  matrix.TransformPoints(expected_output_quad, expectedLocalOutputQuad, 4);

  DlIRect device_filter_ibounds;
  ASSERT_EQ(filter.map_device_bounds(device_input_ibounds, matrix,
                                     device_filter_ibounds),
            &device_filter_ibounds);
  ASSERT_TRUE(containsInclusive(device_filter_ibounds, expected_output_quad));

  DlIRect reverse_input_ibounds;
  ASSERT_EQ(filter.get_input_device_bounds(device_filter_ibounds, matrix,
                                           reverse_input_ibounds),
            &reverse_input_ibounds);
  ASSERT_TRUE(containsInclusive(reverse_input_ibounds, device_input_bounds));
}

static void TestInvalidBounds(const DlImageFilter& filter,
                              const DlTransform& matrix,
                              const DlFRect& localInputBounds) {
  DlIRect device_input_bounds =
      DlIRect::MakeRoundedOut(matrix.TransformRect(localInputBounds));

  DlFRect local_filter_bounds;
  ASSERT_EQ(filter.map_local_bounds(localInputBounds, local_filter_bounds),
            nullptr);
  ASSERT_EQ(local_filter_bounds, localInputBounds);

  DlIRect device_filter_ibounds;
  ASSERT_EQ(filter.map_device_bounds(device_input_bounds, matrix,
                                     device_filter_ibounds),
            nullptr);
  ASSERT_EQ(device_filter_ibounds, device_input_bounds);

  DlIRect reverse_input_ibounds;
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
                       const DlFRect& sourceBounds,
                       const DlFPoint expectedLocalOutputQuad[4]) {
  DlFRect local_filter_bounds;
  ASSERT_EQ(filter.map_local_bounds(sourceBounds, local_filter_bounds),
            &local_filter_bounds);
  ASSERT_TRUE(containsInclusive(local_filter_bounds, expectedLocalOutputQuad));

  for (int scale = 1; scale <= 4; scale++) {
    for (int skew = 0; skew < 8; skew++) {
      for (int degrees = 0; degrees <= 360; degrees += 15) {
        DlTransform matrix;
        // matrix.SetScale(scale, scale);
        // matrix.SkewOuter(skew / 8.0, skew / 8.0);
        // matrix.RotateOuter(DlDegrees(degrees));
        ASSERT_TRUE(matrix.IsInvertible());
        TestBoundsWithMatrix(filter, matrix, sourceBounds,
                             expectedLocalOutputQuad);
        // matrix.SetPerspectiveX(0.001);
        // matrix.SetPerspectiveY(0.001);
        // ASSERT_TRUE(matrix.IsInvertible());
        // TestBoundsWithMatrix(filter, matrix, sourceBounds,
        //                      expectedLocalOutputQuad);
      }
    }
  }
}

static void TestBounds(const DlImageFilter& filter,
                       const DlFRect& sourceBounds,
                       const DlFRect& expectedLocalOutputBounds) {
  DlFPoint expected_local_output_quad[4];
  expectedLocalOutputBounds.GetQuad(expected_local_output_quad);
  TestBounds(filter, sourceBounds, expected_local_output_quad);
}

TEST(DisplayListImageFilter, BlurConstructor) {
  DlBlurImageFilter filter(5.0, 6.0, DlTileMode::kMirror);
}

TEST(DisplayListImageFilter, BlurShared) {
  DlBlurImageFilter filter(5.0, 6.0, DlTileMode::kMirror);

  ASSERT_NE(filter.shared().get(), &filter);
  ASSERT_EQ(*filter.shared(), filter);
}

TEST(DisplayListImageFilter, BlurAsBlur) {
  DlBlurImageFilter filter(5.0, 6.0, DlTileMode::kMirror);

  ASSERT_NE(filter.asBlur(), nullptr);
  ASSERT_EQ(filter.asBlur(), &filter);
}

TEST(DisplayListImageFilter, BlurContents) {
  DlBlurImageFilter filter(5.0, 6.0, DlTileMode::kMirror);

  ASSERT_EQ(filter.sigma_x(), 5.0);
  ASSERT_EQ(filter.sigma_y(), 6.0);
  ASSERT_EQ(filter.tile_mode(), DlTileMode::kMirror);
}

TEST(DisplayListImageFilter, BlurEquals) {
  DlBlurImageFilter filter1(5.0, 6.0, DlTileMode::kMirror);
  DlBlurImageFilter filter2(5.0, 6.0, DlTileMode::kMirror);

  TestEquals(filter1, filter2);
}

TEST(DisplayListImageFilter, BlurWithLocalMatrixEquals) {
  DlBlurImageFilter filter1(5.0, 6.0, DlTileMode::kMirror);
  DlBlurImageFilter filter2(5.0, 6.0, DlTileMode::kMirror);

  DlTransform local_matrix = DlTransform::MakeTranslate(10, 10);
  TestEquals(*filter1.makeWithLocalMatrix(local_matrix),
             *filter2.makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, BlurNotEquals) {
  DlBlurImageFilter filter1(5.0, 6.0, DlTileMode::kMirror);
  DlBlurImageFilter filter2(7.0, 6.0, DlTileMode::kMirror);
  DlBlurImageFilter filter3(5.0, 8.0, DlTileMode::kMirror);
  DlBlurImageFilter filter4(5.0, 6.0, DlTileMode::kRepeat);

  TestNotEquals(filter1, filter2, "Sigma X differs");
  TestNotEquals(filter1, filter3, "Sigma Y differs");
  TestNotEquals(filter1, filter4, "Tile Mode differs");
}

TEST(DisplayListImageFilter, BlurBounds) {
  DlBlurImageFilter filter = DlBlurImageFilter(5, 10, DlTileMode::kDecal);
  DlFRect input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  DlFRect expected_output_bounds = input_bounds.Expand(15, 30);
  TestBounds(filter, input_bounds, expected_output_bounds);
}

TEST(DisplayListImageFilter, BlurZeroSigma) {
  std::shared_ptr<DlImageFilter> filter =
      DlBlurImageFilter::Make(0, 0, DlTileMode::kMirror);
  ASSERT_EQ(filter, nullptr);
  filter = DlBlurImageFilter::Make(3, SK_ScalarNaN, DlTileMode::kMirror);
  ASSERT_EQ(filter, nullptr);
  filter = DlBlurImageFilter::Make(SK_ScalarNaN, 3, DlTileMode::kMirror);
  ASSERT_EQ(filter, nullptr);
  filter =
      DlBlurImageFilter::Make(SK_ScalarNaN, SK_ScalarNaN, DlTileMode::kMirror);
  ASSERT_EQ(filter, nullptr);
  filter = DlBlurImageFilter::Make(3, 0, DlTileMode::kMirror);
  ASSERT_NE(filter, nullptr);
  filter = DlBlurImageFilter::Make(0, 3, DlTileMode::kMirror);
  ASSERT_NE(filter, nullptr);
}

TEST(DisplayListImageFilter, DilateConstructor) {
  DlDilateImageFilter filter(5.0, 6.0);
}

TEST(DisplayListImageFilter, DilateShared) {
  DlDilateImageFilter filter(5.0, 6.0);

  ASSERT_NE(filter.shared().get(), &filter);
  ASSERT_EQ(*filter.shared(), filter);
}

TEST(DisplayListImageFilter, DilateAsDilate) {
  DlDilateImageFilter filter(5.0, 6.0);

  ASSERT_NE(filter.asDilate(), nullptr);
  ASSERT_EQ(filter.asDilate(), &filter);
}

TEST(DisplayListImageFilter, DilateContents) {
  DlDilateImageFilter filter(5.0, 6.0);

  ASSERT_EQ(filter.radius_x(), 5.0);
  ASSERT_EQ(filter.radius_y(), 6.0);
}

TEST(DisplayListImageFilter, DilateEquals) {
  DlDilateImageFilter filter1(5.0, 6.0);
  DlDilateImageFilter filter2(5.0, 6.0);

  TestEquals(filter1, filter2);
}

TEST(DisplayListImageFilter, DilateWithLocalMatrixEquals) {
  DlDilateImageFilter filter1(5.0, 6.0);
  DlDilateImageFilter filter2(5.0, 6.0);

  DlTransform local_matrix = DlTransform::MakeTranslate(10, 10);
  TestEquals(*filter1.makeWithLocalMatrix(local_matrix),
             *filter2.makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, DilateNotEquals) {
  DlDilateImageFilter filter1(5.0, 6.0);
  DlDilateImageFilter filter2(7.0, 6.0);
  DlDilateImageFilter filter3(5.0, 8.0);

  TestNotEquals(filter1, filter2, "Radius X differs");
  TestNotEquals(filter1, filter3, "Radius Y differs");
}

TEST(DisplayListImageFilter, DilateBounds) {
  DlDilateImageFilter filter = DlDilateImageFilter(5, 10);
  DlFRect input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  DlFRect expected_output_bounds = input_bounds.Expand(5, 10);
  TestBounds(filter, input_bounds, expected_output_bounds);
}

TEST(DisplayListImageFilter, ErodeConstructor) {
  DlErodeImageFilter filter(5.0, 6.0);
}

TEST(DisplayListImageFilter, ErodeShared) {
  DlErodeImageFilter filter(5.0, 6.0);

  ASSERT_NE(filter.shared().get(), &filter);
  ASSERT_EQ(*filter.shared(), filter);
}

TEST(DisplayListImageFilter, ErodeAsErode) {
  DlErodeImageFilter filter(5.0, 6.0);

  ASSERT_NE(filter.asErode(), nullptr);
  ASSERT_EQ(filter.asErode(), &filter);
}

TEST(DisplayListImageFilter, ErodeContents) {
  DlErodeImageFilter filter(5.0, 6.0);

  ASSERT_EQ(filter.radius_x(), 5.0);
  ASSERT_EQ(filter.radius_y(), 6.0);
}

TEST(DisplayListImageFilter, ErodeEquals) {
  DlErodeImageFilter filter1(5.0, 6.0);
  DlErodeImageFilter filter2(5.0, 6.0);

  TestEquals(filter1, filter2);
}

TEST(DisplayListImageFilter, ErodeWithLocalMatrixEquals) {
  DlErodeImageFilter filter1(5.0, 6.0);
  DlErodeImageFilter filter2(5.0, 6.0);

  DlTransform local_matrix = DlTransform::MakeTranslate(10, 10);
  TestEquals(*filter1.makeWithLocalMatrix(local_matrix),
             *filter2.makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, ErodeNotEquals) {
  DlErodeImageFilter filter1(5.0, 6.0);
  DlErodeImageFilter filter2(7.0, 6.0);
  DlErodeImageFilter filter3(5.0, 8.0);

  TestNotEquals(filter1, filter2, "Radius X differs");
  TestNotEquals(filter1, filter3, "Radius Y differs");
}

TEST(DisplayListImageFilter, ErodeBounds) {
  DlErodeImageFilter filter = DlErodeImageFilter(5, 10);
  DlFRect input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  DlFRect expected_output_bounds = input_bounds.Expand(-5, -10);
  TestBounds(filter, input_bounds, expected_output_bounds);
}

TEST(DisplayListImageFilter, MatrixConstructor) {
  DlMatrixImageFilter filter(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                       0.5, 3.0, 15),
                             DlImageSampling::kLinear);
}

TEST(DisplayListImageFilter, MatrixShared) {
  DlMatrixImageFilter filter(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                       0.5, 3.0, 15),
                             DlImageSampling::kLinear);

  ASSERT_NE(filter.shared().get(), &filter);
  ASSERT_EQ(*filter.shared(), filter);
}

TEST(DisplayListImageFilter, MatrixAsMatrix) {
  DlMatrixImageFilter filter(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                       0.5, 3.0, 15),
                             DlImageSampling::kLinear);

  ASSERT_NE(filter.asMatrix(), nullptr);
  ASSERT_EQ(filter.asMatrix(), &filter);
}

TEST(DisplayListImageFilter, MatrixContents) {
  DlTransform matrix = DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                 0.5, 3.0, 15);
  DlMatrixImageFilter filter(matrix, DlImageSampling::kLinear);

  ASSERT_EQ(filter.matrix(), matrix);
  ASSERT_EQ(filter.sampling(), DlImageSampling::kLinear);
}

TEST(DisplayListImageFilter, MatrixEquals) {
  DlTransform matrix = DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                 0.5, 3.0, 15);
  DlMatrixImageFilter filter1(matrix, DlImageSampling::kLinear);
  DlMatrixImageFilter filter2(matrix, DlImageSampling::kLinear);

  TestEquals(filter1, filter2);
}

TEST(DisplayListImageFilter, MatrixWithLocalMatrixEquals) {
  DlTransform matrix = DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                 0.5, 3.0, 15);
  DlMatrixImageFilter filter1(matrix, DlImageSampling::kLinear);
  DlMatrixImageFilter filter2(matrix, DlImageSampling::kLinear);

  DlTransform local_matrix = DlTransform::MakeTranslate(10, 10);
  TestEquals(*filter1.makeWithLocalMatrix(local_matrix),
             *filter2.makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, MatrixNotEquals) {
  DlTransform matrix1 = DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                  0.5, 3.0, 15);
  DlTransform matrix2 = DlTransform::MakeAffine2D(5.0, 0.0, 10,  //
                                                  0.5, 3.0, 15);
  DlMatrixImageFilter filter1(matrix1, DlImageSampling::kLinear);
  DlMatrixImageFilter filter2(matrix2, DlImageSampling::kLinear);
  DlMatrixImageFilter filter3(matrix1, DlImageSampling::kNearestNeighbor);

  TestNotEquals(filter1, filter2, "Matrix differs");
  TestNotEquals(filter1, filter3, "Sampling differs");
}

TEST(DisplayListImageFilter, MatrixBounds) {
  DlTransform matrix = DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                 0.5, 3.0, 7);
  ASSERT_TRUE(matrix.IsInvertible());
  DlMatrixImageFilter filter(matrix, DlImageSampling::kLinear);
  DlFRect input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  DlFPoint expectedOutputQuad[4] = {
      {50, 77},    // (20,20) => (20*2 + 10, 20/2 + 20*3 + 7) == (50, 77)
      {50, 257},   // (20,80) => (20*2 + 10, 20/2 + 80*3 + 7) == (50, 257)
      {170, 287},  // (80,80) => (80*2 + 10, 80/2 + 80*3 + 7) == (170, 287)
      {170, 107},  // (80,20) => (80*2 + 10, 80/2 + 20*3 + 7) == (170, 107)
  };
  TestBounds(filter, input_bounds, expectedOutputQuad);
}

TEST(DisplayListImageFilter, ComposeConstructor) {
  DlMatrixImageFilter outer(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                      0.5, 3.0, 15),
                            DlImageSampling::kLinear);
  DlBlurImageFilter inner(5.0, 6.0, DlTileMode::kMirror);
  DlComposeImageFilter filter(outer, inner);
}

TEST(DisplayListImageFilter, ComposeShared) {
  DlMatrixImageFilter outer(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                      0.5, 3.0, 15),
                            DlImageSampling::kLinear);
  DlBlurImageFilter inner(5.0, 6.0, DlTileMode::kMirror);
  DlComposeImageFilter filter(outer, inner);

  ASSERT_NE(filter.shared().get(), &filter);
  ASSERT_EQ(*filter.shared(), filter);
}

TEST(DisplayListImageFilter, ComposeAsCompose) {
  DlMatrixImageFilter outer(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                      0.5, 3.0, 15),
                            DlImageSampling::kLinear);
  DlBlurImageFilter inner(5.0, 6.0, DlTileMode::kMirror);
  DlComposeImageFilter filter(outer, inner);

  ASSERT_NE(filter.asCompose(), nullptr);
  ASSERT_EQ(filter.asCompose(), &filter);
}

TEST(DisplayListImageFilter, ComposeContents) {
  DlMatrixImageFilter outer(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                      0.5, 3.0, 15),
                            DlImageSampling::kLinear);
  DlBlurImageFilter inner(5.0, 6.0, DlTileMode::kMirror);
  DlComposeImageFilter filter(outer, inner);

  ASSERT_EQ(*filter.outer().get(), outer);
  ASSERT_EQ(*filter.inner().get(), inner);
}

TEST(DisplayListImageFilter, ComposeEquals) {
  DlMatrixImageFilter outer1(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                       0.5, 3.0, 15),
                             DlImageSampling::kLinear);
  DlBlurImageFilter inner1(5.0, 6.0, DlTileMode::kMirror);
  DlComposeImageFilter filter1(outer1, inner1);

  DlMatrixImageFilter outer2(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                       0.5, 3.0, 15),
                             DlImageSampling::kLinear);
  DlBlurImageFilter inner2(5.0, 6.0, DlTileMode::kMirror);
  DlComposeImageFilter filter2(outer1, inner1);

  TestEquals(filter1, filter2);
}

TEST(DisplayListImageFilter, ComposeWithLocalMatrixEquals) {
  DlMatrixImageFilter outer1(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                       0.5, 3.0, 15),
                             DlImageSampling::kLinear);
  DlBlurImageFilter inner1(5.0, 6.0, DlTileMode::kMirror);
  DlComposeImageFilter filter1(outer1, inner1);

  DlMatrixImageFilter outer2(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                       0.5, 3.0, 15),
                             DlImageSampling::kLinear);
  DlBlurImageFilter inner2(5.0, 6.0, DlTileMode::kMirror);
  DlComposeImageFilter filter2(outer1, inner1);

  DlTransform local_matrix = DlTransform::MakeTranslate(10, 10);
  TestEquals(*filter1.makeWithLocalMatrix(local_matrix),
             *filter2.makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, ComposeNotEquals) {
  DlMatrixImageFilter outer1(DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                       0.5, 3.0, 15),
                             DlImageSampling::kLinear);
  DlBlurImageFilter inner1(5.0, 6.0, DlTileMode::kMirror);

  DlMatrixImageFilter outer2(DlTransform::MakeAffine2D(5.0, 0.0, 10,  //
                                                       0.5, 3.0, 15),
                             DlImageSampling::kLinear);
  DlBlurImageFilter inner2(7.0, 6.0, DlTileMode::kMirror);

  DlComposeImageFilter filter1(outer1, inner1);
  DlComposeImageFilter filter2(outer2, inner1);
  DlComposeImageFilter filter3(outer1, inner2);

  TestNotEquals(filter1, filter2, "Outer differs");
  TestNotEquals(filter1, filter3, "Inner differs");
}

TEST(DisplayListImageFilter, ComposeBounds) {
  DlDilateImageFilter outer = DlDilateImageFilter(5, 10);
  DlBlurImageFilter inner = DlBlurImageFilter(12, 5, DlTileMode::kDecal);
  DlComposeImageFilter filter = DlComposeImageFilter(outer, inner);
  DlFRect input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  DlFRect expected_output_bounds = input_bounds.Expand(36, 15).Expand(5, 10);
  TestBounds(filter, input_bounds, expected_output_bounds);
}

static void TestUnboundedBounds(DlImageFilter& filter,
                                const DlFRect& sourceBounds,
                                const DlFRect& expectedOutputBounds,
                                const DlFRect& expectedInputBounds) {
  DlFRect bounds;
  EXPECT_EQ(filter.map_local_bounds(sourceBounds, bounds), nullptr);
  EXPECT_EQ(bounds, expectedOutputBounds);

  DlIRect sourceIBounds = DlIRect::MakeRoundedOut(sourceBounds);
  DlIRect ibounds;
  EXPECT_EQ(filter.map_device_bounds(sourceIBounds, DlTransform(), ibounds),
            nullptr);
  DlIRect expectedOutputIBounds = DlIRect::MakeRoundedOut(expectedOutputBounds);
  EXPECT_EQ(ibounds, expectedOutputIBounds);

  EXPECT_EQ(
      filter.get_input_device_bounds(sourceIBounds, DlTransform(), ibounds),
      nullptr);
  DlIRect expectedInputIBounds = DlIRect::MakeRoundedOut(expectedInputBounds);
  EXPECT_EQ(ibounds, expectedInputIBounds);
}

TEST(DisplayListImageFilter, ComposeBoundsWithUnboundedInner) {
  auto input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  auto expected_bounds = DlFRect::MakeLTRB(5, 2, 95, 98);

  DlBlendColorFilter color_filter(DlColor::kRed(), DlBlendMode::kSrcOver);
  auto outer = DlBlurImageFilter(5.0, 6.0, DlTileMode::kRepeat);
  auto inner = DlColorFilterImageFilter(color_filter.shared());
  auto composed = DlComposeImageFilter(outer.shared(), inner.shared());

  TestUnboundedBounds(composed, input_bounds, expected_bounds, expected_bounds);
}

TEST(DisplayListImageFilter, ComposeBoundsWithUnboundedOuter) {
  auto input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  auto expected_bounds = DlFRect::MakeLTRB(5, 2, 95, 98);

  DlBlendColorFilter color_filter(DlColor::kRed(), DlBlendMode::kSrcOver);
  auto outer = DlColorFilterImageFilter(color_filter.shared());
  auto inner = DlBlurImageFilter(5.0, 6.0, DlTileMode::kRepeat);
  auto composed = DlComposeImageFilter(outer.shared(), inner.shared());

  TestUnboundedBounds(composed, input_bounds, expected_bounds, expected_bounds);
}

TEST(DisplayListImageFilter, ComposeBoundsWithUnboundedInnerAndOuter) {
  auto input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  auto expected_bounds = input_bounds;

  DlBlendColorFilter color_filter1(DlColor::kRed(), DlBlendMode::kSrcOver);
  DlBlendColorFilter color_filter2(DlColor::kBlue(), DlBlendMode::kSrcOver);
  auto outer = DlColorFilterImageFilter(color_filter1.shared());
  auto inner = DlColorFilterImageFilter(color_filter2.shared());
  auto composed = DlComposeImageFilter(outer.shared(), inner.shared());

  TestUnboundedBounds(composed, input_bounds, expected_bounds, expected_bounds);
}

// See https://github.com/flutter/flutter/issues/108433
TEST(DisplayListImageFilter, Issue108433) {
  auto input_bounds = DlIRect::MakeLTRB(20, 20, 80, 80);
  auto expected_bounds = DlIRect::MakeLTRB(5, 2, 95, 98);

  DlBlendColorFilter dl_color_filter(DlColor::kRed(), DlBlendMode::kSrcOver);
  auto dl_outer = DlBlurImageFilter(5.0, 6.0, DlTileMode::kRepeat);
  auto dl_inner = DlColorFilterImageFilter(dl_color_filter.shared());
  auto dl_compose = DlComposeImageFilter(dl_outer, dl_inner);

  DlIRect dl_bounds;
  ASSERT_EQ(
      dl_compose.map_device_bounds(input_bounds, DlTransform(), dl_bounds),
      nullptr);
  ASSERT_EQ(dl_bounds, expected_bounds);
}

TEST(DisplayListImageFilter, ColorFilterConstructor) {
  DlBlendColorFilter dl_color_filter(DlColor::kRed(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter(dl_color_filter);
}

TEST(DisplayListImageFilter, ColorFilterShared) {
  DlBlendColorFilter dl_color_filter(DlColor::kRed(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter(dl_color_filter);

  ASSERT_EQ(*filter.shared(), filter);
}

TEST(DisplayListImageFilter, ColorFilterAsColorFilter) {
  DlBlendColorFilter dl_color_filter(DlColor::kRed(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter(dl_color_filter);

  ASSERT_NE(filter.asColorFilter(), nullptr);
  ASSERT_EQ(filter.asColorFilter(), &filter);
}

TEST(DisplayListImageFilter, ColorFilterContents) {
  DlBlendColorFilter dl_color_filter(DlColor::kRed(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter(dl_color_filter);

  ASSERT_EQ(*filter.color_filter().get(), dl_color_filter);
}

TEST(DisplayListImageFilter, ColorFilterEquals) {
  DlBlendColorFilter dl_color_filter1(DlColor::kRed(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter1(dl_color_filter1);

  DlBlendColorFilter dl_color_filter2(DlColor::kRed(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter2(dl_color_filter2);

  TestEquals(filter1, filter2);
}

TEST(DisplayListImageFilter, ColorFilterWithLocalMatrixEquals) {
  DlBlendColorFilter dl_color_filter1(DlColor::kRed(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter1(dl_color_filter1);

  DlBlendColorFilter dl_color_filter2(DlColor::kRed(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter2(dl_color_filter2);

  DlTransform local_matrix = DlTransform::MakeTranslate(10, 10);
  TestEquals(*filter1.makeWithLocalMatrix(local_matrix),
             *filter2.makeWithLocalMatrix(local_matrix));
}

TEST(DisplayListImageFilter, ColorFilterNotEquals) {
  DlBlendColorFilter dl_color_filter1(DlColor::kRed(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter1(dl_color_filter1);

  DlBlendColorFilter dl_color_filter2(DlColor::kBlue(), DlBlendMode::kLighten);
  DlColorFilterImageFilter filter2(dl_color_filter2);

  DlBlendColorFilter dl_color_filter3(DlColor::kRed(), DlBlendMode::kDarken);
  DlColorFilterImageFilter filter3(dl_color_filter3);

  TestNotEquals(filter1, filter2, "Color differs");
  TestNotEquals(filter1, filter3, "Blend Mode differs");
}

TEST(DisplayListImageFilter, ColorFilterBounds) {
  DlBlendColorFilter dl_color_filter(DlColor::kRed(), DlBlendMode::kSrcIn);
  DlColorFilterImageFilter filter(dl_color_filter);
  DlFRect input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  TestBounds(filter, input_bounds, input_bounds);
}

TEST(DisplayListImageFilter, ColorFilterModifiesTransparencyBounds) {
  DlBlendColorFilter dl_color_filter(DlColor::kRed(), DlBlendMode::kSrcOver);
  DlColorFilterImageFilter filter(dl_color_filter);
  DlFRect input_bounds = DlFRect::MakeLTRB(20, 20, 80, 80);
  TestInvalidBounds(filter, DlTransform(), input_bounds);
}

TEST(DisplayListImageFilter, LocalImageFilterBounds) {
  auto sk_filter_matrix = SkMatrix::MakeAll(2.0, 0.0, 10,  //
                                            0.5, 3.0, 15,  //
                                            0.0, 0.0, 1);
  auto dl_filter_matrix = DlTransform::MakeAffine2D(2.0, 0.0, 10,  //
                                                    0.5, 3.0, 15);
  std::vector<sk_sp<SkImageFilter>> sk_filters{
      SkImageFilters::Blur(5.0, 6.0, SkTileMode::kRepeat, nullptr),
      SkImageFilters::ColorFilter(
          SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcOver), nullptr),
      SkImageFilters::Dilate(5.0, 10.0, nullptr),
      SkImageFilters::MatrixTransform(
          sk_filter_matrix, SkSamplingOptions(SkFilterMode::kLinear), nullptr),
      SkImageFilters::Compose(
          SkImageFilters::Blur(5.0, 6.0, SkTileMode::kRepeat, nullptr),
          SkImageFilters::ColorFilter(
              SkColorFilters::Blend(SK_ColorRED, SkBlendMode::kSrcOver),
              nullptr))};

  DlBlendColorFilter dl_color_filter(DlColor::kRed(), DlBlendMode::kSrcOver);
  std::vector<std::shared_ptr<DlImageFilter>> dl_filters{
      std::make_shared<DlBlurImageFilter>(5.0, 6.0, DlTileMode::kRepeat),
      std::make_shared<DlColorFilterImageFilter>(dl_color_filter.shared()),
      std::make_shared<DlDilateImageFilter>(5, 10),
      std::make_shared<DlMatrixImageFilter>(dl_filter_matrix,
                                            DlImageSampling::kLinear),
      std::make_shared<DlComposeImageFilter>(
          std::make_shared<DlBlurImageFilter>(5.0, 6.0, DlTileMode::kRepeat),
          std::make_shared<DlColorFilterImageFilter>(
              dl_color_filter.shared()))};

  DlTransform persp = DlTransform::MakeRowMajor(
      // clang-format off
      1.0f, 0.0f,   0.0f, 0.0f,
      0.0f, 1.0f,   0.0f, 0.0f,
      0.0f, 0.0f,   1.0f, 0.0f,
      0.0f, 0.001f, 0.0f, 1.0f
      // clang-format on
  );
  std::vector<DlTransform> matrices = {
      DlTransform::MakeTranslate(10.0, 10.0),
      DlTransform::MakeScale(2.0, 2.0).TranslateInner(10.0, 10.0),
      DlTransform::MakeRotate(DlDegrees(45)).TranslateInner(5.0, 5.0),
      persp,
  };
  std::vector<DlTransform> bounds_matrices{
      DlTransform::MakeTranslate(5.0, 10.0),
      DlTransform::MakeScale(2.0, 2.0),
  };

  for (unsigned i = 0; i < sk_filters.size(); i++) {
    for (unsigned j = 0; j < matrices.size(); j++) {
      for (unsigned k = 0; k < bounds_matrices.size(); k++) {
        auto desc = "filter " + std::to_string(i + 1)             //
                    + ", filter matrix " + std::to_string(j + 1)  //
                    + ", bounds matrix " + std::to_string(k + 1);
        auto& m = matrices[j];
        auto& bounds_matrix = bounds_matrices[k];
        auto sk_local_filter =
            sk_filters[i]->makeWithLocalMatrix(m.ToSkMatrix());
        auto dl_local_filter = dl_filters[i]->makeWithLocalMatrix(m);
        if (!sk_local_filter || !dl_local_filter) {
          // Temporarily relax the equivalence testing to allow Skia to expand
          // their behavior. Once the Skia fixes are rolled in, the
          // DlImageFilter should adapt  to the new rules.
          // See https://github.com/flutter/flutter/issues/114723
          FML_LOG(ERROR) << "matrix = " << m;
          FML_LOG(ERROR) << "sk filter: " << sk_local_filter;
          FML_LOG(ERROR) << "dl filter: " << dl_local_filter;
          ASSERT_TRUE(sk_local_filter || !dl_local_filter) << desc;
          continue;
        }
        {
          auto sk_input_bounds = SkIRect::MakeLTRB(20, 20, 80, 80);
          auto dl_input_bounds = DlIRect::MakeBounds(sk_input_bounds);
          SkIRect sk_rect;
          DlIRect dl_rect;
          sk_rect = sk_local_filter->filterBounds(
              sk_input_bounds, bounds_matrix.ToSkMatrix(),
              SkImageFilter::MapDirection::kForward_MapDirection);
          if (dl_local_filter->map_device_bounds(dl_input_bounds, bounds_matrix,
                                                 dl_rect)) {
            ASSERT_EQ(DlIRect::MakeBounds(sk_rect), dl_rect) << desc;
          } else {
            ASSERT_TRUE(dl_local_filter->modifies_transparent_black()) << desc;
            ASSERT_FALSE(sk_local_filter->canComputeFastBounds()) << desc;
          }
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
          auto sk_outset_bounds = SkIRect::MakeLTRB(20, 20, 80, 80);
          auto dl_outset_bounds = DlIRect::MakeBounds(sk_outset_bounds);
          SkIRect sk_rect;
          DlIRect dl_rect;
          sk_rect = sk_local_filter->filterBounds(
              sk_outset_bounds, bounds_matrix.ToSkMatrix(),
              SkImageFilter::MapDirection::kReverse_MapDirection);
          if (dl_local_filter->get_input_device_bounds(
                  dl_outset_bounds, bounds_matrix, dl_rect)) {
            ASSERT_EQ(DlIRect::MakeBounds(sk_rect), dl_rect) << desc;
          } else {
            ASSERT_TRUE(dl_local_filter->modifies_transparent_black());
            ASSERT_FALSE(sk_local_filter->canComputeFastBounds());
          }
        }
      }
    }
  }
}

}  // namespace testing
}  // namespace flutter
