// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_

#include <algorithm>
#include <limits>

#include "flutter/display_list/dl_base_types.h"

namespace flutter {

template <typename T, typename LT>
struct DlRectT {
  T left;
  T top;
  T right;
  T bottom;

  DlRectT() : DlRectT(0, 0, 0, 0) {}
  DlRectT(T left, T top, T right, T bottom)
      : left(left), top(top), right(right), bottom(bottom) {}
  DlRectT(const DlRectT& r)
      : left(r.left), top(r.top), right(r.right), bottom(r.bottom) {}
  DlRectT(DlRectT&& r)
      : left(r.left), top(r.top), right(r.right), bottom(r.bottom) {}

  DlRectT MakeLTRB(T left, T top, T right, T bottom) {
    return {left, top, right, bottom};
  }

  DlRectT MakeXYWH(T x, T y, LT w, LT h) {
    return {x, y, x+w, y+h};
  }

  LT width() const { return right - left; }
  LT height() const { return bottom - top; }

  bool is_empty() const { return !(right > left && bottom > top); }

  void Join(const DlRectT* r) {
    if (!r->is_empty()) {
      if (is_empty()) {
        *this = *r;
      } else {
        left = std::min(left, r->left);
        top = std::min(top, r->top);
        right = std::max(right, r->right);
        bottom = std::max(bottom, r->bottom);
      }
    }
  }
  void Join(const DlRectT& r) { join(&r); }

  bool Intersect(const DlRectT* r) {
    if (is_empty()) {
      return false;
    }
    if (r->is_empty()) {
      *this = {0,0,0,0};
      return false;
    }
    left = std::max(left, r->left);
    top = std::max(top, r->top);
    right = std::min(right, r->right);
    bottom = std::min(bottom, r->bottom);
    return !is_empty();
  }
  bool Intersect(const DlRectT& r) { return intersect(&r); }

  bool Intersects(const DlRectT* r) const {
    return !this->is_empty() &&
           this->left < r->right && r->left < this->right &&
           this->top < r->bottom && r->top < this->bottom;
  }
  bool Intersects(const DlRectT& r) const { return intersects(&r); }

  bool Contains(const DlRectT* r) const {
    return !this->empty() &&
           this->left <= r->left && this->right >= r->right &&
           this->top <= r->top && this->bottom >= r->bottom;
  }
  bool Contains(const DlRectT& r) const { return contains(&r); }
};

using DlRectF = DlRectT<DlScalar, DlScalar>;
using DlRectI = DlRectT<DlInt, DlLong>;

DlRectI RoundOut(const DlRectF* r) {
  return {
      static_cast<DlInt>(floor(r->left)),
      static_cast<DlInt>(floor(r->top)),
      static_cast<DlInt>(ceil(r->right)),
      static_cast<DlInt>(ceil(r->bottom)),
  };
}

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_
