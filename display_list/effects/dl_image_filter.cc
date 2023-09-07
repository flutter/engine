// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_image_filter.h"

namespace flutter {

std::shared_ptr<DlImageFilter> DlImageFilter::makeWithLocalMatrix(
    const DlTransform& matrix) const {
  if (matrix.IsIdentity()) {
    return shared();
  }
  // Matrix
  switch (this->matrix_capability()) {
    case MatrixCapability::kTranslate: {
      if (!matrix.IsTranslate()) {
        // Nothing we can do at this point
        return nullptr;
      }
      break;
    }
    case MatrixCapability::kScaleTranslate: {
      if (!matrix.IsScaleTranslate()) {
        // Nothing we can do at this point
        return nullptr;
      }
      break;
    }
    default:
      break;
  }
  return std::make_shared<DlLocalMatrixImageFilter>(matrix, shared());
}

DlFRect* DlComposeImageFilter::map_local_bounds(const DlFRect& input_bounds,
                                                DlFRect& output_bounds) const {
  DlFRect cur_bounds = input_bounds;
  DlFRect* ret = &output_bounds;
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

DlIRect* DlComposeImageFilter::map_device_bounds(const DlIRect& input_bounds,
                                                 const DlTransform& ctm,
                                                 DlIRect& output_bounds) const {
  DlIRect cur_bounds = input_bounds;
  DlIRect* ret = &output_bounds;
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

DlIRect* DlComposeImageFilter::get_input_device_bounds(
    const DlIRect& output_bounds,
    const DlTransform& ctm,
    DlIRect& input_bounds) const {
  DlIRect cur_bounds = output_bounds;
  DlIRect* ret = &input_bounds;
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
