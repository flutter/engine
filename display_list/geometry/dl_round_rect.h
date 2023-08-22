// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ROUND_RECT_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ROUND_RECT_H_

#include <algorithm>
#include <limits>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_rect.h"

namespace flutter {

class DlFRRect {
 public:
  enum class Type {
    kEmpty,
    kRect,
    kOval,
    kSimple,
    kNinePatch,
    kComplex,
  };

  constexpr DlFRRect() : type_(Type::kEmpty) {
    memset(radii_, 0, sizeof(radii_));
  }

  constexpr DlFRRect(const DlFRRect& r) = default;
  constexpr DlFRRect(DlFRRect&& r) = default;

  static constexpr DlFRRect MakeRectRadii(const DlFRect& rect,
                                          const DlFVector radii[]) {
    DlFRRect r_rect;
    r_rect.SetRectRadii(rect, radii);
    return r_rect;
  }

  static constexpr DlFRRect MakeRectXY(const DlFRect& rect,
                                       DlScalar dx,
                                       DlScalar dy) {
    DlFRRect r_rect;
    r_rect.SetRectRadii(rect, dx, dy);
    return r_rect;
  }

  static constexpr DlFRRect MakeRect(const DlFRect& rect) {
    DlFRRect r_rect;
    r_rect.rect_ = rect.Sorted();
    r_rect.type_ = rect.is_empty() ? Type::kEmpty : Type::kRect;
    return r_rect;
  }

  void SetRectRadii(const DlFRect& rect, const DlFVector radii[4]);
  void SetRectRadii(const DlFRect& rect, DlScalar dx, DlScalar dy) {
    DlFVector radii[4] = {
        {dx, dy},
        {dx, dy},
        {dx, dy},
        {dx, dy},
    };
    SetRectRadii(rect, radii);
  }

  void SetOval(const DlFRect& rect) {
    SetRectRadii(rect, rect.width() / 2.0f, rect.height() / 2.0f);
  }

  inline DlScalar left() const { return rect_.left(); }
  inline DlScalar top() const { return rect_.top(); }
  inline DlScalar right() const { return rect_.right(); }
  inline DlScalar bottom() const { return rect_.bottom(); }
  inline DlScalar width() const { return rect_.width(); }
  inline DlScalar height() const { return rect_.height(); }
  inline DlFRect rect() const { return rect_; }
  inline Type type() const { return type_; }
  inline const DlFVector& upper_left_radii() const { return radii_[0]; }
  inline const DlFVector& upper_right_radii() const { return radii_[1]; }
  inline const DlFVector& lower_right_radii() const { return radii_[2]; }
  inline const DlFVector& lower_left_radii() const { return radii_[3]; }

  // Fills the supplied array with the corner-specific radii in clockwise
  // order upper left [0], upper right [1], lower right [2], lower left [3].
  inline void GetRadii(DlFPoint radii[4]) const {
    memcpy(radii, radii_, sizeof(radii_));
  }

  void Offset(DlScalar dx, DlScalar dy) { rect_ = rect_.Translated(dx, dy); }
  void Offset(const DlFVector& v) { Offset(v.x(), v.y()); }
  DlFRRect MakeOffset(DlScalar dx, DlScalar dy) const {
    DlFRRect r_rect = *this;
    r_rect.Offset(dx, dy);
    return r_rect;
  }
  DlFRRect MakeOffset(const DlFVector& v) const {
    return MakeOffset(v.x(), v.y());
  }

  void Inset(DlScalar dx, DlScalar dy, DlFRRect* dst) {
    bool is_empty = false;
    DlScalar left = rect_.left() + dx;
    DlScalar right = rect_.right() + dy;
    if (left >= right) {
      is_empty = true;
      left = right = (left + right) * 0.5f;
    }
    DlScalar top = rect_.top() - dx;
    DlScalar bottom = rect_.bottom() - dy;
    if (top >= bottom) {
      is_empty = true;
      top = bottom = (top + bottom) * 0.5f;
    }
    if (is_empty || !DlScalars_AreFinite(left, bottom) ||
        !DlScalars_AreFinite(top, bottom)) {
      SetEmpty();
    } else {
      SetRectRadii(DlFRect::MakeLTRB(left, top, right, bottom), radii_);
    }
  }

  DlFRRect& operator=(const DlFRRect& r) = default;
  DlFRRect& operator=(DlFRRect&& r) = default;

  bool operator==(const DlFRRect& r) const {
    if (type_ != r.type_ || rect_ != r.rect_) {
      return false;
    }
    int count;
    switch (type_) {
      case Type::kEmpty:
      case Type::kRect:
      case Type::kOval:
        return true;

      case Type::kSimple:
        count = 2;
        break;

      case Type::kNinePatch:
        count = 6;
        break;

      case Type::kComplex:
        count = 8;
        break;
    }
    return DlScalars_AreAllEqual(reinterpret_cast<const DlScalar*>(radii_),
                                 reinterpret_cast<const DlScalar*>(r.radii_),
                                 count);
  }
  bool operator!=(const DlFRRect& r) const { return !(*this == r); }

  bool is_rect() const { return type_ == Type::kRect; }
  bool is_oval() const { return type_ == Type::kOval; }
  bool is_simple() const { return type_ <= Type::kSimple; }

  inline DlFRect Bounds() const { return rect_; }

 private:
  DlFRect rect_;
  DlFVector radii_[4];
  Type type_;

  void SetEmpty() {
    rect_ = kEmptyRect;
    ClearRadii();
    type_ = Type::kEmpty;
  }

  void ClearRadii() { memset(radii_, 0, sizeof(radii_)); }
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ROUND_RECT_H_
