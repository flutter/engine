// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_SIZE_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_SIZE_H_

#include <algorithm>
#include <limits>
#include <ostream>

#include "flutter/display_list/dl_base_types.h"

namespace flutter {

template <typename T>
struct DlTSize {
 private:
  static constexpr T zero_ = static_cast<T>(0);

  T width_;
  T height_;

 public:
  constexpr DlTSize() : DlTSize(zero_, zero_) {}
  constexpr DlTSize(T width, T height) : width_(width), height_(height) {}

  template <typename U>
  static constexpr DlTSize MakeSize(const U& size) {
    return {static_cast<T>(size.width()), static_cast<T>(size.height())};
  }

  constexpr T width() const { return width_; }
  constexpr T height() const { return height_; }
  constexpr T area() const { return width_ * height_; }

  constexpr bool is_empty() const { return !(width_ >= 0 && height_ >= 0); }
  constexpr bool operator==(const DlTSize& p) const {
    return width_ == p.width_ && height_ == p.height_;
  }
  constexpr bool operator!=(const DlTSize& p) const { return !(*this == p); }
};

using DlFSize = DlTSize<DlScalar>;
using DlISize = DlTSize<DlSize>;

template <typename T>
inline std::ostream& operator<<(std::ostream& os, const DlTSize<T>& size) {
  return os << "DlPoint(" << size.width() << ", " << size.width() << ")";
}

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_SIZE_H_
