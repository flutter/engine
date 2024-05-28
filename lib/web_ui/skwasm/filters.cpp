// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"

using namespace Skwasm;

SKWASM_EXPORT ImageFilter* imageFilter_createBlur(Scalar sigmaX,
                                                    Scalar sigmaY,
                                                    TileMode tileMode) {
  return ImageFilters::Blur(sigmaX, sigmaY, tileMode, nullptr).release();
}

SKWASM_EXPORT ImageFilter* imageFilter_createDilate(Scalar radiusX,
                                                      Scalar radiusY) {
  return ImageFilters::Dilate(radiusX, radiusY, nullptr).release();
}

SKWASM_EXPORT ImageFilter* imageFilter_createErode(Scalar radiusX,
                                                     Scalar radiusY) {
  return ImageFilters::Erode(radiusX, radiusY, nullptr).release();
}

SKWASM_EXPORT ImageFilter* imageFilter_createMatrix(Scalar* matrix33,
                                                      FilterQuality quality) {
  return ImageFilters::MatrixTransform(createMatrix(matrix33),
                                         samplingOptionsForQuality(quality),
                                         nullptr)
      .release();
}

SKWASM_EXPORT ImageFilter* imageFilter_createFromColorFilter(
    ColorFilter* filter) {
  return ImageFilters::ColorFilter(sk_ref_sp<ColorFilter>(filter), nullptr)
      .release();
}

SKWASM_EXPORT ImageFilter* imageFilter_compose(ImageFilter* outer,
                                                 ImageFilter* inner) {
  return ImageFilters::Compose(sk_ref_sp<ImageFilter>(outer),
                                 sk_ref_sp<ImageFilter>(inner))
      .release();
}

SKWASM_EXPORT void imageFilter_dispose(ImageFilter* filter) {
  filter->unref();
}

SKWASM_EXPORT void imageFilter_getFilterBounds(ImageFilter* filter,
                                               IRect* inOutBounds) {
  IRect outputRect =
      filter->filterBounds(*inOutBounds, SkMatrix(),
                           ImageFilter::MapDirection::kForward_MapDirection);
  *inOutBounds = outputRect;
}

SKWASM_EXPORT ColorFilter* colorFilter_createMode(Color color,
                                                    SkBlendMode mode) {
  return ColorFilters::Blend(color, mode).release();
}

SKWASM_EXPORT ColorFilter* colorFilter_createMatrix(
    float* matrixData  // 20 values
) {
  return ColorFilters::Matrix(matrixData).release();
}

SKWASM_EXPORT ColorFilter* colorFilter_createSRGBToLinearGamma() {
  return ColorFilters::SRGBToLinearGamma().release();
}

SKWASM_EXPORT ColorFilter* colorFilter_createLinearToSRGBGamma() {
  return ColorFilters::LinearToSRGBGamma().release();
}

SKWASM_EXPORT ColorFilter* colorFilter_compose(ColorFilter* outer,
                                                 ColorFilter* inner) {
  return ColorFilters::Compose(sk_ref_sp<ColorFilter>(outer),
                                 sk_ref_sp<ColorFilter>(inner))
      .release();
}

SKWASM_EXPORT void colorFilter_dispose(ColorFilter* filter) {
  filter->unref();
}

SKWASM_EXPORT MaskFilter* maskFilter_createBlur(SkBlurStyle blurStyle,
                                                  Scalar sigma) {
  return MaskFilter::MakeBlur(blurStyle, sigma).release();
}

SKWASM_EXPORT void maskFilter_dispose(MaskFilter* filter) {
  filter->unref();
}
