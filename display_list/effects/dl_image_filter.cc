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

dl_shared<const DlImageFilter> DlImageFilter::MakeBlur(SkScalar sigma_x,
                                                       SkScalar sigma_y,
                                                       DlTileMode tile_mode) {
  return DlBlurImageFilter::Make(sigma_x, sigma_y, tile_mode);
}

dl_shared<DlBlurImageFilter> DlBlurImageFilter::Make(SkScalar sigma_x,
                                                     SkScalar sigma_y,
                                                     DlTileMode tile_mode) {
  if (checkAndClampPositive(sigma_x, sigma_y)) {
    return dl_shared(new DlBlurImageFilter(sigma_x, sigma_y, tile_mode));
  }
  return nullptr;
}

dl_shared<const DlImageFilter> DlImageFilter::MakeDilate(SkScalar radius_x,
                                                         SkScalar radius_y) {
  return DlDilateImageFilter::Make(radius_x, radius_y);
}

dl_shared<DlDilateImageFilter> DlDilateImageFilter::Make(SkScalar radius_x,
                                                         SkScalar radius_y) {
  if (checkAndClampPositive(radius_x, radius_y)) {
    return dl_shared(new DlDilateImageFilter(radius_x, radius_y));
  }
  return nullptr;
}

dl_shared<const DlImageFilter> DlImageFilter::MakeErode(SkScalar radius_x,
                                                        SkScalar radius_y) {
  return DlErodeImageFilter::Make(radius_x, radius_y);
}

dl_shared<DlErodeImageFilter> DlErodeImageFilter::Make(SkScalar radius_x,
                                                       SkScalar radius_y) {
  if (checkAndClampPositive(radius_x, radius_y)) {
    return dl_shared(new DlErodeImageFilter(radius_x, radius_y));
  }
  return nullptr;
}

dl_shared<const DlImageFilter> DlImageFilter::MakeMatrix(
    const SkMatrix& matrix,
    DlImageSampling sampling) {
  return DlMatrixImageFilter::Make(matrix, sampling);
}

dl_shared<DlMatrixImageFilter> DlMatrixImageFilter::Make(
    const SkMatrix& matrix,
    DlImageSampling sampling) {
  if (matrix.isFinite() && !matrix.isIdentity()) {
    return dl_shared(new DlMatrixImageFilter(matrix, sampling));
  }
  return nullptr;
}

dl_shared<const DlImageFilter> DlImageFilter::MakeCompose(
    const dl_shared<const DlImageFilter>& outer,
    const dl_shared<const DlImageFilter>& inner) {
  return DlComposeImageFilter::Make(outer, inner);
}

dl_shared<const DlImageFilter> DlComposeImageFilter::Make(
    const dl_shared<const DlImageFilter>& outer,
    const dl_shared<const DlImageFilter>& inner) {
  if (!outer) {
    return inner;
  }
  if (!inner) {
    return outer;
  }
  return dl_shared(new DlComposeImageFilter(outer, inner));
}

dl_shared<const DlImageFilter> DlImageFilter::MakeColorFilter(
    const dl_shared<const DlColorFilter>& filter) {
  return DlColorFilterImageFilter::Make(filter);
}

dl_shared<DlColorFilterImageFilter> DlColorFilterImageFilter::Make(
    const dl_shared<const DlColorFilter>& filter) {
  if (filter) {
    return dl_shared(new DlColorFilterImageFilter(filter));
  }
  return nullptr;
}

dl_shared<DlLocalMatrixImageFilter> DlLocalMatrixImageFilter::Make(
    const SkMatrix& matrix,
    const dl_shared<const DlImageFilter>& filter) {
  return dl_shared(new DlLocalMatrixImageFilter(matrix, filter));
}

dl_shared<const DlImageFilter> DlImageFilter::makeWithLocalMatrix(
    const SkMatrix& matrix) const {
  if (matrix.isIdentity()) {
    return this;
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
  return DlLocalMatrixImageFilter::Make(matrix, this);
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
