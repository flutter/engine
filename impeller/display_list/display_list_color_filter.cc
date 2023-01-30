// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/display_list_color_filter.h"
#include "impeller/aiks/color_filter_factory.h"
#include "impeller/display_list/conversion.h"
#include "impeller/entity/contents/color_source_contents.h"

namespace impeller {

DlBlendColorFilterFactory::DlBlendColorFilterFactory(
    const flutter::DlColorFilter* filter)
    : filter_(filter->asBlend()) {}

DlBlendColorFilterFactory::~DlBlendColorFilterFactory() {}

ColorFilterFactory::ColorSourceType DlBlendColorFilterFactory::GetType() const {
  return ColorFilterFactory::ColorSourceType::kBlend;
}

std::shared_ptr<ColorFilterContents> DlBlendColorFilterFactory::MakeContents(
    FilterInput::Ref input) const {
  auto blend_mode = ToBlendMode(filter_->mode());
  auto color = ToColor(filter_->color());
  return ColorFilterContents::MakeBlend(blend_mode, {std::move(input)}, color);
}

/// DlMatrixColorFilterFactory

DlMatrixColorFilterFactory::DlMatrixColorFilterFactory(
    const flutter::DlColorFilter* filter)
    : filter_(filter->asMatrix()) {}

DlMatrixColorFilterFactory::~DlMatrixColorFilterFactory() {}

ColorFilterFactory::ColorSourceType DlMatrixColorFilterFactory::GetType()
    const {
  return ColorFilterFactory::ColorSourceType::kMatrix;
}

std::shared_ptr<ColorFilterContents> DlMatrixColorFilterFactory::MakeContents(
    FilterInput::Ref input) const {
  const flutter::DlMatrixColorFilter* dl_matrix = filter_->asMatrix();
  impeller::FilterContents::ColorMatrix color_matrix;
  dl_matrix->get_matrix(color_matrix.array);
  return ColorFilterContents::MakeColorMatrix({std::move(input)}, color_matrix);
}

/// DlSrgbToLinearColorFilterFactory

DlSrgbToLinearColorFilterFactory::DlSrgbToLinearColorFilterFactory() {}

DlSrgbToLinearColorFilterFactory::~DlSrgbToLinearColorFilterFactory() {}

ColorFilterFactory::ColorSourceType DlSrgbToLinearColorFilterFactory::GetType()
    const {
  return ColorFilterFactory::ColorSourceType::kSrgbToLinearGamma;
}

std::shared_ptr<ColorFilterContents>
DlSrgbToLinearColorFilterFactory::MakeContents(FilterInput::Ref input) const {
  return ColorFilterContents::MakeSrgbToLinearFilter({std::move(input)});
}

/// DlLinearToSrgbColorFilterFactory

DlLinearToSrgbColorFilterFactory::DlLinearToSrgbColorFilterFactory() {}

DlLinearToSrgbColorFilterFactory::~DlLinearToSrgbColorFilterFactory() {}

ColorFilterFactory::ColorSourceType DlLinearToSrgbColorFilterFactory::GetType()
    const {
  return ColorFilterFactory::ColorSourceType::kLinearToSrgbGamma;
}

std::shared_ptr<ColorFilterContents>
DlLinearToSrgbColorFilterFactory::MakeContents(FilterInput::Ref input) const {
  return ColorFilterContents::MakeLinearToSrgbFilter({std::move(input)});
}

}  // namespace impeller
