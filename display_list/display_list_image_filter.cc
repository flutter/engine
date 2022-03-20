// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_image_filter.h"

namespace flutter {

std::shared_ptr<DlImageFilter> DlImageFilter::From(SkImageFilter* sk_filter) {
  if (sk_filter == nullptr) {
    return nullptr;
  }
  {
    SkColorFilter* color_filter;
    if (sk_filter->asAColorFilter(&color_filter)) {
      FML_DCHECK(color_filter != nullptr);
      return std::make_shared<DlColorFilterImageFilter>(
          DlColorFilter::From(sk_ref_sp(color_filter)));
    }
  }
  return std::make_shared<DlUnknownImageFilter>(sk_ref_sp(sk_filter));
}

}  // namespace flutter
