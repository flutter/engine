// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/display_list_color_source.h"

namespace flutter {

std::shared_ptr<DlColorSource> DlColorSource::From(SkShader* sk_shader) {
  if (sk_shader == nullptr) {
    return nullptr;
  }
  {
    SkMatrix local_matrix;
    SkTileMode xy[2];
    SkImage* image = sk_shader->isAImage(&local_matrix, xy);
    if (image) {
      DlTileMode h_mode = static_cast<DlTileMode>(xy[0]);
      DlTileMode v_mode = static_cast<DlTileMode>(xy[1]);
      return std::make_shared<DlImageColorSource>(sk_ref_sp(image),
                                                  local_matrix, h_mode, v_mode);
    }
  }
  // It may look like the Gradients can be recaptured from the Skia objects
  // via the |SkShader->asAGradient(&info)| method, but the info object they
  // fill has a number of parameters which are missing, including the local
  // matrix in every gradient, and the sweep angles in the sweep gradients.
  //
  // As we cannot know if we are recapturing any of the gradients accurately
  // given the lack of information about the local matrix, we will treat them
  // all as "unknown".
  return std::make_shared<DlUnknownColorSource>(sk_ref_sp(sk_shader));
}

std::shared_ptr<DlColorSource> DlColorSource::MakeLinear(const SkPoint p0,
                                          const SkPoint p1,
                                          uint32_t stop_count,
                                          const uint32_t* colors,
                                          const float* stops,
                                          DlTileMode tile_mode,
                                          const SkMatrix& matrix) {
  size_t needed = sizeof(DlLinearGradientColorSource) +
                  (stop_count * (sizeof(uint32_t) + sizeof(float)));

  void* storage = ::operator new (needed);

  std::shared_ptr<DlLinearGradientColorSource> ret;
  ret.reset(new (storage) DlLinearGradientColorSource(p0, p1, stop_count, tile_mode, matrix));
  memcpy(ret->unsafe_colors_array(), colors, stop_count * sizeof(*colors));
  memcpy(ret->unsafe_stops_array(), stops, stop_count * sizeof(*stops));
  return std::move(ret);
}

}  // namespace flutter
