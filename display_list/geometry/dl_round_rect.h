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
#include "flutter/fml/logging.h"

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

  static DlFRRect MakeRectRadii(const DlFRect& rect, const DlFVector radii[4]);

  static DlFRRect MakeRectXY(const DlFRect& rect, DlScalar dx, DlScalar dy);

  static DlFRRect MakeRectXY(const DlFRect& rect, DlFVector radii) {
    return MakeRectXY(rect, radii.x(), radii.y());
  }

  static constexpr DlFRRect MakeRect(const DlFRect& rect) {
    if (rect.is_empty()) {
      return DlFRRect();
    }
    return DlFRRect(rect);
  }

  static constexpr DlFRRect MakeOval(const DlFRect& bounds) {
    DlFRect sorted = bounds.Sorted();
    return MakeRectXY(sorted, sorted.width() / 2.0f, sorted.height() / 2.0f);
  }

  inline DlScalar left() const { return rect_.left(); }
  inline DlScalar top() const { return rect_.top(); }
  inline DlScalar right() const { return rect_.right(); }
  inline DlScalar bottom() const { return rect_.bottom(); }
  inline DlScalar x() const { return rect_.x(); }
  inline DlScalar y() const { return rect_.y(); }
  inline DlScalar width() const { return rect_.width(); }
  inline DlScalar height() const { return rect_.height(); }
  inline const DlFRect& rect() const { return rect_; }
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

  [[nodiscard]] DlFRRect Translated(DlScalar dx, DlScalar dy) const {
    return MakeRectRadii(rect_.Translated(dx, dy), radii_);
  }
  [[nodiscard]] DlFRRect Translated(const DlFVector& v) const {
    return Translated(v.x(), v.y());
  }

  [[nodiscard]] DlFRRect Padded(DlScalar dx, DlScalar dy) const {
    DlScalar left = rect_.left() - dx;
    DlScalar right = rect_.right() + dy;
    if (!(left < right)) {
      return DlFRRect();
    }
    DlScalar top = rect_.top() - dx;
    DlScalar bottom = rect_.bottom() + dy;
    if (!(top < bottom)) {
      return DlFRRect();
    }
    return MakeRectRadii(DlFRect::MakeLTRB(left, top, right, bottom), radii_);
  }
  [[nodiscard]] DlFRRect Padded(DlFVector inset) const {
    return Padded(inset.x(), inset.y());
  }

  bool Contains(const DlFPoint& p) const;
  bool Contains(const DlFRect& r) const;

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

  bool is_empty() const { return type_ == Type::kEmpty; }
  bool is_rect() const { return type_ == Type::kRect; }
  bool is_oval() const { return type_ == Type::kOval; }
  bool is_simple() const { return type_ == Type::kSimple; }
  bool is_nine_patch() const { return type_ == Type::kNinePatch; }
  bool is_complex() const { return type_ == Type::kComplex; }

  bool is_finite() const {
    if (!rect_.is_finite()) {
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
    return DlScalars_AreAllFinite(reinterpret_cast<const DlScalar*>(radii_),
                                  count);
  }

  inline const DlFRect& Bounds() const { return rect_; }

 private:
  DlFRRect(const DlFRect& rect, const DlFVector radii[4], Type type)
      : rect_(rect), type_(type) {
    memcpy(radii_, radii, sizeof(radii_));
  }

  DlFRRect(const DlFRect& rect, Type type = Type::kRect)
      : rect_(rect), type_(type) {
    FML_DCHECK(type == Type::kRect || type == Type::kOval);
    memset(radii_, 0, sizeof(radii_));
  }

  DlFRect rect_;
  DlFVector radii_[4];
  Type type_;
};

std::ostream& operator<<(std::ostream& os, const DlFRRect& rrect);

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_ROUND_RECT_H_
