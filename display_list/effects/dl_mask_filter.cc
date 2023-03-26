// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/effects/dl_mask_filter.h"

namespace flutter {

dl_shared<DlMaskFilter> DlMaskFilter::MakeBlur(DlBlurStyle style,
                                               SkScalar sigma,
                                               bool respect_ctm) {
  return DlBlurMaskFilter::Make(style, sigma, respect_ctm);
}

dl_shared<DlBlurMaskFilter> DlBlurMaskFilter::Make(DlBlurStyle style,
                                                   SkScalar sigma,
                                                   bool respect_ctm) {
  if (SkScalarIsFinite(sigma) && sigma > 0) {
    return dl_shared(new DlBlurMaskFilter(style, sigma, respect_ctm));
  }
  return nullptr;
}

}  // namespace flutter
