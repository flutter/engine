// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_image_filter.h"

namespace flutter {

static bool checkAndClampPositive(SkScalar& value1, SkScalar& value2) {
  if (SkScalarIsFinite(value1) && SkScalarIsFinite(value2)) {
    bool positive = false;
    if (value1 < SK_ScalarNearlyZero) {
      value1 = 0.0f;
    } else {
      positive = true;
    }
    if (value2 < SK_ScalarNearlyZero) {
      value2 = 0.0f;
    } else {
      positive = true;
    }
    return positive;
  }
  return false;
}

std::shared_ptr<const DlImageFilter> DlImageFilter::MakeBlur(
    SkScalar sigma_x,
    SkScalar sigma_y,
    DlTileMode tile_mode) {
  return DlBlurImageFilter::Make(sigma_x, sigma_y, tile_mode);
}

std::shared_ptr<DlBlurImageFilter> DlBlurImageFilter::Make(
    SkScalar sigma_x,
    SkScalar sigma_y,
    DlTileMode tile_mode) {
  if (checkAndClampPositive(sigma_x, sigma_y)) {
    return std::shared_ptr<DlBlurImageFilter>(
        new DlBlurImageFilter(sigma_x, sigma_y, tile_mode));
  }
  return nullptr;
}

std::shared_ptr<const DlImageFilter> DlImageFilter::MakeDilate(
    SkScalar radius_x,
    SkScalar radius_y) {
  return DlDilateImageFilter::Make(radius_x, radius_y);
}

std::shared_ptr<DlDilateImageFilter> DlDilateImageFilter::Make(
    SkScalar radius_x,
    SkScalar radius_y) {
  if (checkAndClampPositive(radius_x, radius_y)) {
    return std::shared_ptr<DlDilateImageFilter>(
        new DlDilateImageFilter(radius_x, radius_y));
  }
  return nullptr;
}

std::shared_ptr<const DlImageFilter> DlImageFilter::MakeErode(
    SkScalar radius_x,
    SkScalar radius_y) {
  return DlErodeImageFilter::Make(radius_x, radius_y);
}

std::shared_ptr<DlErodeImageFilter> DlErodeImageFilter::Make(
    SkScalar radius_x,
    SkScalar radius_y) {
  if (checkAndClampPositive(radius_x, radius_y)) {
    return std::shared_ptr<DlErodeImageFilter>(
        new DlErodeImageFilter(radius_x, radius_y));
  }
  return nullptr;
}

std::shared_ptr<const DlImageFilter> DlImageFilter::MakeMatrix(
    const SkMatrix& matrix,
    DlImageSampling sampling) {
  return DlMatrixImageFilter::Make(matrix, sampling);
}

std::shared_ptr<DlMatrixImageFilter> DlMatrixImageFilter::Make(
    const SkMatrix& matrix,
    DlImageSampling sampling) {
  if (matrix.isFinite() && !matrix.isIdentity()) {
    return std::shared_ptr<DlMatrixImageFilter>(
        new DlMatrixImageFilter(matrix, sampling));
  }
  return nullptr;
}

std::shared_ptr<const DlImageFilter> DlImageFilter::MakeCompose(
    const std::shared_ptr<const DlImageFilter>& outer,
    const std::shared_ptr<const DlImageFilter>& inner) {
  return DlComposeImageFilter::Make(outer, inner);
}

std::shared_ptr<const DlImageFilter> DlComposeImageFilter::Make(
    const std::shared_ptr<const DlImageFilter>& outer,
    const std::shared_ptr<const DlImageFilter>& inner) {
  if (!outer) {
    return inner;
  }
  if (!inner) {
    return outer;
  }
  return std::shared_ptr<DlComposeImageFilter>(
      new DlComposeImageFilter(outer, inner));
}

std::shared_ptr<const DlImageFilter> DlImageFilter::MakeColorFilter(
    const std::shared_ptr<const DlColorFilter>& filter) {
  return DlColorFilterImageFilter::Make(filter);
}

std::shared_ptr<DlColorFilterImageFilter> DlColorFilterImageFilter::Make(
    const std::shared_ptr<const DlColorFilter>& filter) {
  if (filter) {
    return std::shared_ptr<DlColorFilterImageFilter>(
        new DlColorFilterImageFilter(filter));
  }
  return nullptr;
}

std::shared_ptr<DlLocalMatrixImageFilter> DlLocalMatrixImageFilter::Make(
    const SkMatrix& matrix,
    const std::shared_ptr<const DlImageFilter>& filter) {
  return std::shared_ptr<DlLocalMatrixImageFilter>(
      new DlLocalMatrixImageFilter(matrix, filter));
}

std::shared_ptr<const DlImageFilter> DlImageFilter::makeWithLocalMatrix(
    const SkMatrix& matrix) const {
  if (matrix.isIdentity()) {
    return shared_from_this();
  }
  // Matrix
  switch (this->matrix_capability()) {
    case MatrixCapability::kTranslate: {
      if (!matrix.isTranslate()) {
        // Nothing we can do at this point
        return nullptr;
      }
      break;
    }
    case MatrixCapability::kScaleTranslate: {
      if (!matrix.isScaleTranslate()) {
        // Nothing we can do at this point
        return nullptr;
      }
      break;
    }
    default:
      break;
  }
  return DlLocalMatrixImageFilter::Make(matrix, shared_from_this());
}

SkRect* DlComposeImageFilter::map_local_bounds(const SkRect& input_bounds,
                                               SkRect& output_bounds) const {
  SkRect cur_bounds = input_bounds;
  SkRect* ret = &output_bounds;
  // We set this result in case neither filter is present.
  output_bounds = input_bounds;
  if (inner_) {
    if (!inner_->map_local_bounds(cur_bounds, output_bounds)) {
      ret = nullptr;
    }
    cur_bounds = output_bounds;
  }
  if (outer_) {
    if (!outer_->map_local_bounds(cur_bounds, output_bounds)) {
      ret = nullptr;
    }
  }
  return ret;
}

SkIRect* DlComposeImageFilter::map_device_bounds(const SkIRect& input_bounds,
                                                 const SkMatrix& ctm,
                                                 SkIRect& output_bounds) const {
  SkIRect cur_bounds = input_bounds;
  SkIRect* ret = &output_bounds;
  // We set this result in case neither filter is present.
  output_bounds = input_bounds;
  if (inner_) {
    if (!inner_->map_device_bounds(cur_bounds, ctm, output_bounds)) {
      ret = nullptr;
    }
    cur_bounds = output_bounds;
  }
  if (outer_) {
    if (!outer_->map_device_bounds(cur_bounds, ctm, output_bounds)) {
      ret = nullptr;
    }
  }
  return ret;
}

SkIRect* DlComposeImageFilter::get_input_device_bounds(
    const SkIRect& output_bounds,
    const SkMatrix& ctm,
    SkIRect& input_bounds) const {
  SkIRect cur_bounds = output_bounds;
  SkIRect* ret = &input_bounds;
  // We set this result in case neither filter is present.
  input_bounds = output_bounds;
  if (outer_) {
    if (!outer_->get_input_device_bounds(cur_bounds, ctm, input_bounds)) {
      ret = nullptr;
    }
    cur_bounds = input_bounds;
  }
  if (inner_) {
    if (!inner_->get_input_device_bounds(cur_bounds, ctm, input_bounds)) {
      ret = nullptr;
    }
  }
  return ret;
}

}  // namespace flutter
