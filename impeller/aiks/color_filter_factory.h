// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "impeller/entity/contents/filters/color_filter_contents.h"

namespace impeller {

class ColorFilterFactory {
 public:
  enum class ColorSourceType {
    kBlend,
    kMatrix,
    kSrgbToLinearGamma,
    kLinearToSrgbGamma,
  };

  virtual ~ColorFilterFactory();

  virtual std::shared_ptr<ColorFilterContents> MakeContents(
      FilterInput::Ref input) const = 0;

  virtual ColorSourceType GetType() const = 0;

  virtual bool Equal(const ColorFilterFactory& other) const = 0;
};

}  // namespace impeller