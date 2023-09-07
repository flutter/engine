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
    /// The bounds of the round rect are empty so all other attributes of its
    /// geometry are irrelevant.
    kEmpty,

    /// The round rect is a non-empty rectangle with no rounded corners at all.
    kRect,

    /// The round rect is a perfect non-empty oval (or circle) where the
    /// rounded corners come together and meet on the sides leaving no flat
    /// areas on the edges.
    kOval,

    /// The round rect is non-empty and has flat areas on at least 2 of its
    /// sides with all four corners having the same circular profile. All 8
    /// radii of the round rect will be identical and smaller than either the
    /// width or the height or both.
    kCircularCorners,

    /// The round rect is non-empty and has flat areas on at least 2 of its
    /// sides with all four corners having the same oval (but not circular)
    /// profile - i.e. with different vertical and horizontal radii. Each
    /// corner is a mirror reflection to the corner above/below or left/right
    /// of it.
    kOvalCorners,

    /// The round rect is non-empty and has flat areas on at least 2 of its
    /// sides with each radius being a mirror copy of the same radius to
    /// its left/right or above/below. The 8 individual horizontal and vertical
    /// radii will match in the following 4 pairings:
    /// - UL vertical radius == UR vertical radius
    /// - UR horizontal radius == LR horizontal radius
    /// - LR vertical radius == LL vertical radius
    /// - LL horizontal radius == UL horizontal radius
    /// Note that ever corner appears twice in those pairings, but its vertical
    /// and horizontal radii will each match different adjacent corners.
    kNinePatch,

    /// The round rect is non-empty and has 8 radii on its 4 different corners
    /// that may or may not match individually, but which do not follow any of
    /// the previously identified patterns.
    kComplex,
  };

  constexpr DlFRRect() : type_(Type::kEmpty) {
    memset(radii_, 0, sizeof(radii_));
  }

  constexpr DlFRRect(const DlFRRect& r) = default;
  constexpr DlFRRect(DlFRRect&& r) = default;

  DlFRRect& operator=(const DlFRRect& r) = default;
  DlFRRect& operator=(DlFRRect&& r) = default;

  static DlFRRect MakeRectRadii(const DlFRect& rect, const DlFVector radii[4]);

  static DlFRRect MakeRectXY(const DlFRect& rect, DlScalar dx, DlScalar dy);

  static DlFRRect MakeRectXY(const DlFRect& rect, DlFVector radii) {
    return MakeRectXY(rect, radii.x(), radii.y());
  }

  static constexpr DlFRRect MakeRect(const DlFRect& rect) {
    if (rect.IsEmpty()) {
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

  /// The round rect matches the criteria given for |Type::kEmpty| above.
  bool IsEmpty() const { return type_ == Type::kEmpty; }

  /// The round rect matches the criteria given for |Type::kRect| above.
  bool IsRect() const { return type_ == Type::kRect; }

  /// The round rect matches the criteria given for |Type::kOval| above.
  bool IsOval() const { return type_ == Type::kOval; }

  /// The round rect matches the criteria given for |Type::CircularCorners|
  /// above.
  bool HasCircularCorners() const { return type_ == Type::kCircularCorners; }

  /// The round rect matches the criteria given for either of the
  /// |Type::kCircularCorners| or |Type::kOvalCorners| types above.
  /// The circular corner case will match this property as well since
  /// a circle is a degenerate oval, while the |type()| property will
  /// return the exact type, distinguishing circular from oval corners.
  bool HasOvalCorners() const {
    return type_ == Type::kCircularCorners ||  //
           type_ == Type::kOvalCorners;
  }

  // The round rect matches the criteria given for |Type::kNinePatch| above.
  bool IsNinePatch() const { return type_ == Type::kNinePatch; }

  // The round rect matches the criteria given for |Type::kComplex| above.
  bool IsComplex() const { return type_ == Type::kComplex; }

  // Fills the supplied array with the corner-specific radii in clockwise
  // order upper left [0], upper right [1], lower right [2], lower left [3].
  inline void GetRadii(DlFPoint radii[4]) const {
    memcpy(radii, radii_, sizeof(radii_));
  }

  [[nodiscard]] DlFRRect Translate(DlScalar dx, DlScalar dy) const {
    return MakeRectRadii(rect_.Translate(dx, dy), radii_);
  }
  [[nodiscard]] DlFRRect Translate(const DlFVector& v) const {
    return Translate(v.x(), v.y());
  }

  [[nodiscard]] DlFRRect Expand(DlScalar dx, DlScalar dy) const;
  [[nodiscard]] DlFRRect Expand(DlFVector inset) const {
    return Expand(inset.x(), inset.y());
  }

  bool Contains(const DlFPoint& p) const;
  bool Contains(const DlFRect& r) const;

  bool operator==(const DlFRRect& r) const;
  bool operator!=(const DlFRRect& r) const { return !(*this == r); }

  bool IsFinite() const;

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
