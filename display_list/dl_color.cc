// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/dl_color.h"

namespace flutter {

DlColor DlColor::withColorSpace(DlColorSpace color_space) const {
  switch (color_space_) {
    case DlColorSpace::kSRGB:
      switch (color_space) {
        case DlColorSpace::kSRGB:
          return *this;
        case DlColorSpace::kExtendedSRGB:
          return DlColor(alpha_, red_, green_, blue_,
                         DlColorSpace::kExtendedSRGB);
        case DlColorSpace::kDisplayP3:
          FML_CHECK(false) << "not implemented";
          return *this;
      }
    case DlColorSpace::kExtendedSRGB:
      switch (color_space) {
        case DlColorSpace::kSRGB:
          FML_CHECK(false) << "not implemented";
          return *this;
        case DlColorSpace::kExtendedSRGB:
          return *this;
        case DlColorSpace::kDisplayP3:
          FML_CHECK(false) << "not implemented";
          return *this;
      }
    case DlColorSpace::kDisplayP3:
      switch (color_space) {
        case DlColorSpace::kSRGB:
          FML_CHECK(false) << "not implemented";
          return *this;
        case DlColorSpace::kExtendedSRGB:
          FML_CHECK(false) << "not implemented";
          return *this;
        case DlColorSpace::kDisplayP3:
          return *this;
      }
  }
}

}  // namespace flutter
