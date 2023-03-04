// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/skia/dl_sk_utils.h"

#include <math.h>
#include <optional>
#include <type_traits>

#include "flutter/display_list/display_list_blend_mode.h"
#include "flutter/fml/logging.h"

namespace flutter {

// clang-format off
constexpr float kInvertColorMatrix[20] = {
  -1.0,    0,    0, 1.0, 0,
     0, -1.0,    0, 1.0, 0,
     0,    0, -1.0, 1.0, 0,
   1.0,  1.0,  1.0, 1.0, 0
};
// clang-format on

void SkPaintDispatchHelper::save_opacity(SkScalar child_opacity) {
  save_stack_.emplace_back(opacity_);
  set_opacity(child_opacity);
}
void SkPaintDispatchHelper::restore_opacity() {
  if (save_stack_.empty()) {
    return;
  }
  set_opacity(save_stack_.back().opacity);
  save_stack_.pop_back();
}

void SkPaintDispatchHelper::setAntiAlias(bool aa) {
  paint_.setAntiAlias(aa);
}
void SkPaintDispatchHelper::setDither(bool dither) {
  paint_.setDither(dither);
}
void SkPaintDispatchHelper::setInvertColors(bool invert) {
  invert_colors_ = invert;
  paint_.setColorFilter(makeColorFilter());
}
void SkPaintDispatchHelper::setStrokeCap(DlStrokeCap cap) {
  paint_.setStrokeCap(ToSk(cap));
}
void SkPaintDispatchHelper::setStrokeJoin(DlStrokeJoin join) {
  paint_.setStrokeJoin(ToSk(join));
}
void SkPaintDispatchHelper::setStyle(DlDrawStyle style) {
  paint_.setStyle(ToSk(style));
}
void SkPaintDispatchHelper::setStrokeWidth(SkScalar width) {
  paint_.setStrokeWidth(width);
}
void SkPaintDispatchHelper::setStrokeMiter(SkScalar limit) {
  paint_.setStrokeMiter(limit);
}
void SkPaintDispatchHelper::setColor(DlColor color) {
  current_color_ = color;
  paint_.setColor(color);
  if (has_opacity()) {
    paint_.setAlphaf(paint_.getAlphaf() * opacity());
  }
}
void SkPaintDispatchHelper::setBlendMode(DlBlendMode mode) {
  paint_.setBlendMode(ToSk(mode));
}
void SkPaintDispatchHelper::setColorSource(const DlColorSource* source) {
  paint_.setShader(source ? source->skia_object() : nullptr);
}
void SkPaintDispatchHelper::setImageFilter(const DlImageFilter* filter) {
  paint_.setImageFilter(filter ? filter->skia_object() : nullptr);
}
void SkPaintDispatchHelper::setColorFilter(const DlColorFilter* filter) {
  color_filter_ = filter ? filter->shared() : nullptr;
  paint_.setColorFilter(makeColorFilter());
}
void SkPaintDispatchHelper::setPathEffect(const DlPathEffect* effect) {
  paint_.setPathEffect(effect ? effect->skia_object() : nullptr);
}
void SkPaintDispatchHelper::setMaskFilter(const DlMaskFilter* filter) {
  paint_.setMaskFilter(filter ? filter->skia_object() : nullptr);
}

sk_sp<SkColorFilter> SkPaintDispatchHelper::makeColorFilter() const {
  if (!invert_colors_) {
    return color_filter_ ? color_filter_->skia_object() : nullptr;
  }
  sk_sp<SkColorFilter> invert_filter =
      SkColorFilters::Matrix(kInvertColorMatrix);
  if (color_filter_) {
    invert_filter = invert_filter->makeComposed(color_filter_->skia_object());
  }
  return invert_filter;
}

}  // namespace flutter
