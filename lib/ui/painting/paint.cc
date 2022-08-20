// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/paint.h"

#include "flutter/display_list/display_list_builder.h"
#include "flutter/fml/logging.h"
#include "flutter/lib/ui/painting/color_filter.h"
#include "flutter/lib/ui/painting/image_filter.h"
#include "flutter/lib/ui/painting/shader.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkString.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"
#include "third_party/tonic/typed_data/typed_list.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(ui, Paint);

// A color matrix which inverts colors.
// clang-format off
constexpr float kInvertColors[20] = {
  -1.0,    0,    0, 1.0, 0,
     0, -1.0,    0, 1.0, 0,
     0,    0, -1.0, 1.0, 0,
   1.0,  1.0,  1.0, 1.0, 0
};
// clang-format on

Paint::Paint(bool enable_dither) : is_dither_(enable_dither) {}

Paint::~Paint() = default;

const SkPaint* Paint::paint(SkPaint& paint) const {
  FML_DCHECK(paint == SkPaint());
  paint.setAntiAlias(is_anti_alias_);
  paint.setDither(is_dither_);

  paint.setColor(color_);
  paint.setBlendMode(static_cast<SkBlendMode>(blend_mode_));
  paint.setStyle(static_cast<SkPaint::Style>(style_));
  paint.setStrokeCap(static_cast<SkPaint::Cap>(stroke_cap_));
  paint.setStrokeJoin(static_cast<SkPaint::Join>(stroke_join_));
  paint.setStrokeWidth(stroke_width_);
  paint.setStrokeMiter(stroke_miter_);

  // Must be done before invert colors, in case we have to compose filters.
  paint.setColorFilter(sk_color_filter());
  if (is_invert_colors_) {
    sk_sp<SkColorFilter> invert_filter = SkColorFilters::Matrix(kInvertColors);
    sk_sp<SkColorFilter> current_filter = paint.refColorFilter();
    if (current_filter) {
      invert_filter = invert_filter->makeComposed(current_filter);
    }
    paint.setColorFilter(invert_filter);
  }
  paint.setImageFilter(sk_image_filter());
  paint.setMaskFilter(sk_mask_filter());
  paint.setShader(sk_shader());

  return &paint;
}

bool Paint::sync_to(DisplayListBuilder* builder,
                    const DisplayListAttributeFlags& flags) const {
  if (flags.applies_shader()) {
    builder->setColorSource(dl_color_source().get());
  }

  if (flags.applies_color_filter()) {
    builder->setColorFilter(dl_color_filter().get());
    builder->setInvertColors(is_invert_colors_);
  }

  if (flags.applies_image_filter()) {
    builder->setImageFilter(dl_image_filter().get());
  }

  if (flags.applies_anti_alias()) {
    builder->setAntiAlias(is_anti_alias_);
  }

  if (flags.applies_alpha_or_color()) {
    builder->setColor(DlColor(color_));
  }

  if (flags.applies_blend()) {
    builder->setBlendMode(static_cast<DlBlendMode>(blend_mode_));
  }

  if (flags.applies_style()) {
    builder->setStyle(static_cast<DlDrawStyle>(style_));
  }

  if (flags.is_stroked(builder->getStyle())) {
    builder->setStrokeWidth(stroke_width_);
    builder->setStrokeMiter(stroke_miter_);
    builder->setStrokeCap(static_cast<DlStrokeCap>(stroke_cap_));
    builder->setStrokeJoin(static_cast<DlStrokeJoin>(stroke_join_));
  }

  if (flags.applies_dither()) {
    builder->setDither(is_dither_);
  }

  if (flags.applies_path_effect()) {
    // The paint API exposed to Dart does not support path effects.  But other
    // operations such as text may set a path effect, which must be cleared.
    builder->setPathEffect(nullptr);
  }

  if (flags.applies_mask_filter()) {
    if (!mask_filter_.has_value()) {
      builder->setMaskFilter(nullptr);
    } else {
      builder->setMaskFilter(&mask_filter_.value());
    }
  }

  return true;
}

}  // namespace flutter
