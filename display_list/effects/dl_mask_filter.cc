// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_mask_filter.h"

namespace flutter {

std::shared_ptr<DlMaskFilter> DlMaskFilter::MakeBlur(DlBlurStyle style,
                                                     SkScalar sigma,
                                                     bool respect_ctm) {
  return DlBlurMaskFilter::Make(style, sigma, respect_ctm);
}

std::shared_ptr<DlBlurMaskFilter> DlBlurMaskFilter::Make(DlBlurStyle style,
                                                         SkScalar sigma,
                                                         bool respect_ctm) {
  if (SkScalarIsFinite(sigma) && sigma > 0) {
    return std::shared_ptr<DlBlurMaskFilter>(
        new DlBlurMaskFilter(style, sigma, respect_ctm));
  }
  return nullptr;
}

}  // namespace flutter
