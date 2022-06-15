// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/paint.h"

#include <memory>

#include "impeller/entity/contents/box_shadow_contents.h"
#include "impeller/entity/contents/solid_color_contents.h"
#include "impeller/entity/contents/solid_stroke_contents.h"

namespace impeller {

std::shared_ptr<Contents> Paint::CreateContentsForEntity(Path path,
                                                         bool cover) const {
  if (contents) {
    contents->SetPath(std::move(path));
    return contents;
  }

  switch (style) {
    case Style::kFill: {
      if (mask_blur.has_value() &&
          mask_blur->style == FilterContents::BlurStyle::kNormal &&
          path.IsRectangle()) {
        auto box_shadow = std::make_shared<BoxShadowContents>();
        box_shadow->SetRect(path.GetBoundingBox());
        box_shadow->SetColor(color);
        box_shadow->SetSigma(mask_blur->sigma);
        return WithFilters(box_shadow, false);
      }

      auto solid_color = std::make_shared<SolidColorContents>();
      solid_color->SetPath(std::move(path));
      solid_color->SetColor(color);
      solid_color->SetCover(cover);
      return WithFilters(solid_color, true, true);
    }
    case Style::kStroke: {
      auto solid_stroke = std::make_shared<SolidStrokeContents>();
      solid_stroke->SetPath(std::move(path));
      solid_stroke->SetColor(color);
      solid_stroke->SetStrokeSize(stroke_width);
      solid_stroke->SetStrokeMiter(stroke_miter);
      solid_stroke->SetStrokeCap(stroke_cap);
      solid_stroke->SetStrokeJoin(stroke_join);
      return WithFilters(solid_stroke, true, true);
    }
  }

  return nullptr;
}

std::shared_ptr<Contents> Paint::WithFilters(
    std::shared_ptr<Contents> input,
    bool with_mask_filter,
    std::optional<bool> is_solid_color) const {
  bool is_solid_color_val = is_solid_color.value_or(!contents);

  if (with_mask_filter && mask_blur.has_value()) {
    if (is_solid_color_val) {
      input = FilterContents::MakeGaussianBlur(
          FilterInput::Make(input), mask_blur->sigma, mask_blur->sigma,
          mask_blur->style);
    } else {
      input = FilterContents::MakeBorderMaskBlur(
          FilterInput::Make(input), mask_blur->sigma, mask_blur->sigma,
          mask_blur->style);
    }
  }

  if (image_filter.has_value()) {
    const ImageFilterProc& filter = image_filter.value();
    input = filter(FilterInput::Make(input));
  }

  if (color_filter.has_value()) {
    const ColorFilterProc& filter = color_filter.value();
    input = filter(FilterInput::Make(input));
  }

  return input;
}

}  // namespace impeller
