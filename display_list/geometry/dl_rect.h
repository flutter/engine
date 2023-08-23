// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_

#include <algorithm>
#include <limits>
#include <optional>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_size.h"

namespace flutter {

#define DL_ONLY_ON_FLOAT(Type) DL_ONLY_ON_FLOAT_M(, Type)
#define DL_ONLY_ON_FLOAT_M(Modifiers, Type) \
  template <typename U = T>                 \
  Modifiers std::enable_if_t<std::is_floating_point_v<U>, Type>

#define DL_ONLY_FROM_FLOAT(FT, Type) DL_ONLY_FROM_FLOAT_M(FT, , Type)
#define DL_ONLY_FROM_FLOAT_M(FT, Modifiers, Type) \
  template <typename FT>                          \
  Modifiers std::enable_if_t<std::is_floating_point_v<FT>, Type>

#define DL_ONLY_INT_FROM_FLOAT(Type) DL_ONLY_INT_FROM_FLOAT_M(IT, FT, Type)
#define DL_ONLY_INT_FROM_FLOAT_M(IT, FT, Modifiers, Type) \
  template <typename IT = T, typename FT>                 \
  Modifiers std::enable_if_t<                             \
      std::is_integral_v<IT> && std::is_floating_point_v<FT>, Type>

template <typename T, typename ST>
struct DlTRect {
 private:
  static constexpr T zero_ = static_cast<T>(0);

  T left_;
  T top_;
  T right_;
  T bottom_;

  constexpr DlTRect(T left, T top, T right, T bottom)
      : left_(left), top_(top), right_(right), bottom_(bottom) {}

  template <typename = std::enable_if<std::is_integral_v<T>>>
  static T SafeAdd(T location, ST distance) {
    if (distance <= static_cast<ST>(0)) {
      return location;
    }
    T result = location + distance;
    if (result < location) {
      return std::numeric_limits<T>::max();
    }
    return result;
  }

  static inline T SafeAdd(T location, ST distance) {
    return location + distance;
  }

 public:
  constexpr DlTRect() : DlTRect(zero_, zero_, zero_, zero_) {}

  constexpr DlTRect(const DlTRect& r) = default;
  constexpr DlTRect(DlTRect&& r) = default;

  static constexpr DlTRect MakeLTRB(T left, T top, T right, T bottom) {
    return {left, top, right, bottom};
  }

  static constexpr DlTRect MakeXYWH(T x, T y, ST w, ST h) {
    return {x, y, SafeAdd(x, w), SafeAdd(y, h)};
  }

  static constexpr DlTRect MakeWH(T w, T h) {
    return MakeLTRB(zero_, zero_, w, h);
  }

  template <typename U>
  static constexpr DlTRect MakeSize(DlTSize<U> size) {
    return MakeLTRB(zero_, zero_, size.width(), size.height());
  }

  static constexpr DlTRect MakeOriginSize(DlTPoint<T> origin,
                                          DlTSize<ST> size) {
    return MakeXYWH(origin.x(), origin.y(), size.width(), size.height());
  }

  template <typename U>
  static constexpr DlTRect MakeBounds(const U& r) {
    return MakeLTRB(r.left(), r.top(), r.right(), r.bottom());
  }

  DL_ONLY_FROM_FLOAT_M(FT, static constexpr, DlTRect)
  MakeRoundedOut(const DlTRect<FT, FT>& r) {
    return MakeLTRB(                      //
        static_cast<T>(floor(r.left())),  //
        static_cast<T>(floor(r.top())),   //
        static_cast<T>(ceil(r.right())),  //
        static_cast<T>(ceil(r.bottom()))  //
    );
  }

  DL_ONLY_FROM_FLOAT_M(FT, static constexpr, DlTRect)
  MakeRoundedIn(const DlTRect<FT, FT>& r) {
    return MakeLTRB(                       //
        static_cast<T>(ceil(r.left())),    //
        static_cast<T>(ceil(r.top())),     //
        static_cast<T>(floor(r.right())),  //
        static_cast<T>(floor(r.bottom()))  //
    );
  }

  DL_ONLY_FROM_FLOAT_M(FT, static constexpr, DlTRect)
  MakeRounded(const DlTRect<FT, FT>& r) {
    return MakeLTRB(                       //
        static_cast<T>(round(r.left())),   //
        static_cast<T>(round(r.top())),    //
        static_cast<T>(round(r.right())),  //
        static_cast<T>(round(r.bottom()))  //
    );
  }

  DlTRect& operator=(const DlTRect& r) = default;
  DlTRect& operator=(DlTRect&& r) = default;

  constexpr inline T left() const { return left_; }
  constexpr inline T top() const { return top_; }
  constexpr inline T right() const { return right_; }
  constexpr inline T bottom() const { return bottom_; }

  constexpr inline T x() const { return left_; }
  constexpr inline T y() const { return top_; }
  constexpr inline DlTPoint<T> origin() const {
    return DlTPoint<T>(left_, top_);
  }

#define DL_RECT_DIM(min, max) min <= max ? max - min : zero_
  constexpr inline ST width() const { return DL_RECT_DIM(left_, right_); }
  constexpr inline ST height() const { return DL_RECT_DIM(top_, bottom_); }
  constexpr inline DlTSize<ST> size() const {
    return DlTSize<ST>(width(), height());
  }
#undef DL_RECT_DIM

  constexpr inline bool is_empty() const {
    return !(right_ > left_ && bottom_ > top_);
  }

  DL_ONLY_ON_FLOAT(bool)
  constexpr is_finite() const { return DlScalars_AreAllFinite(&left_, 4); }

  // Returns the 4 corners of the rectangle in the order UL, UR, LR, LL
  void GetQuad(DlTPoint<T> quad[4]) const {
    quad[0] = {left_, top_};
    quad[1] = {right_, top_};
    quad[2] = {right_, bottom_};
    quad[3] = {left_, bottom_};
  }

  bool operator==(const DlTRect& r) const {
    return left_ == r.left_ &&    //
           top_ == r.top_ &&      //
           right_ == r.right_ &&  //
           bottom_ == r.bottom_;
  }
  bool operator!=(const DlTRect& r) const { return !(*this == r); }

#define DL_SORT(OUTA, OUTB, INA, INB) \
  do {                                \
    if (INA <= INB) {                 \
      OUTA = INA;                     \
      OUTB = INB;                     \
    } else {                          \
      OUTA = INB;                     \
      OUTB = INA;                     \
    }                                 \
  } while (0)

  [[nodiscard]] constexpr DlTRect Sorted() const {
    T l, r, t, b;
    DL_SORT(l, r, left_, right_);
    DL_SORT(t, b, top_, bottom_);
    return MakeLTRB(l, t, r, b);
  }
#undef DL_SORT

#define DL_ROUNDED_UL_LR(ULF, LRF) \
  MakeLTRB((ULF)(left_), (ULF)(top_), (LRF)(right_), (LRF)(bottom_))

  DL_ONLY_ON_FLOAT_M([[nodiscard]] constexpr, DlTRect)
  RoundedOut() const { return DL_ROUNDED_UL_LR(floor, ceil); }

  DL_ONLY_ON_FLOAT_M([[nodiscard]] constexpr, DlTRect)
  RoundedIn() const { return DL_ROUNDED_UL_LR(ceil, floor); }

  DL_ONLY_ON_FLOAT_M([[nodiscard]] constexpr, DlTRect)
  Rounded() const { return DL_ROUNDED_UL_LR(round, round); }

#undef DL_ROUNDED_UL_LR

  [[nodiscard]] DlTPoint<T> Center() const {
    return DlTPoint<T>((left_ + right_) / static_cast<T>(2),
                       (top_ + bottom_) / static_cast<T>(2));
  }

  [[nodiscard]] DlTRect UpperLeftQuadrant() const {
    auto center = Center();
    return {left_, top_, center.x(), center.y()};
  }

  [[nodiscard]] DlTRect UpperRightQuadrant() const {
    auto center = Center();
    return {center.x(), top_, right_, center.y()};
  }

  [[nodiscard]] DlTRect LowerLeftQuadrant() const {
    auto center = Center();
    return {left_, center.y(), center.x(), bottom_};
  }

  [[nodiscard]] DlTRect LowerRightQuadrant() const {
    auto center = Center();
    return {center.x(), center.y(), right_, bottom_};
  }

  [[nodiscard]] DlTRect Padded(T marginX, T marginY) const {
    return {
        left_ - marginX,
        top_ - marginY,
        right_ + marginX,
        bottom_ + marginY,
    };
  }
  [[nodiscard]] DlTRect Padded(DlTPoint<T> v) const {
    return Padded(v.x(), v.y());
  }

  [[nodiscard]] DlTRect Translated(T dx, T dy) const {
    return {
        left_ + dx,
        top_ + dy,
        right_ + dx,
        bottom_ + dy,
    };
  }
  [[nodiscard]] DlTRect Translated(DlTPoint<T> v) const {
    return Translated(v.x(), v.y());
  }

  DlTRect Union(const DlTRect& r) const {
    if (r.is_empty()) {
      return *this;
    }
    if (is_empty()) {
      return r;
    }
    return MakeLTRB(                  //
        std::min(left_, r.left_),     //
        std::min(top_, r.top_),       //
        std::max(right_, r.right_),   //
        std::max(bottom_, r.bottom_)  //
    );
  }
  DlTRect Union(const std::optional<DlTRect>& r) const {
    return r.has_value() ? Union(r.value()) : *this;
  }

  std::optional<DlTRect> Intersection(const DlTRect& r) const {
    if (!Intersects(r)) {
      return std::nullopt;
    }
    return MakeLTRB(                  //
        std::max(left_, r.left_),     //
        std::max(top_, r.top_),       //
        std::min(right_, r.right_),   //
        std::min(bottom_, r.bottom_)  //
    );
  }
  std::optional<DlTRect> Intersection(const std::optional<DlTRect>& r) const {
    return r.has_value() ? Intersection(r.value()) : std::nullopt;
  }
  DlTRect IntersectionOrEmpty(const DlTRect& r) const {
    return Intersection(r).value_or(DlTRect());
  }
  DlTRect IntersectionOrEmpty(const std::optional<DlTRect>& r) const {
    return Intersection(r).value_or(DlTRect());
  }

  bool Intersects(const DlTRect& r) const {
    return !this->is_empty() && !r.is_empty() &&  //
           this->left_ < r.right_ && r.left_ < this->right_ &&
           this->top_ < r.bottom_ && r.top_ < this->bottom_;
  }

  bool Contains(T x, T y) const {
    return !this->is_empty() && DlScalars_AreFinite(x, y) &&  //
           x >= left_ && x < right_ &&                        //
           y >= top_ && y < bottom_;
  }
  bool Contains(DlTPoint<T> p) const { return Contains(p.x(), p.y()); }

  bool Contains(const DlTRect& r) const {
    return !this->is_empty() && !r.is_empty() &&  //
           this->left_ <= r.left_ && this->right_ >= r.right_ &&
           this->top_ <= r.top_ && this->bottom_ >= r.bottom_;
  }
};

#undef DL_ONLY_ON_FLOAT
#undef DL_ONLY_ON_FLOAT_M
#undef DL_ONLY_FROM_FLOAT
#undef DL_ONLY_FROM_FLOAT_M
#undef DL_ONLY_INT_FROM_FLOAT
#undef DL_ONLY_INT_FROM_FLOAT_M

using DlIRect = DlTRect<DlInt, DlSize>;
using DlFRect = DlTRect<DlScalar, DlScalar>;

template <typename T, typename ST>
inline std::ostream& operator<<(std::ostream& os, const DlTRect<T, ST>& rect) {
  return os << "DlRect(" << rect.left() << ", " << rect.top() << " => "
            << rect.right() << ", " << rect.bottom() << ")";
}

[[maybe_unused]] constexpr DlFRect kMaxCullRect =
    DlFRect::MakeLTRB(-1E9F, -1E9F, 1E9F, 1E9F);

[[maybe_unused]] constexpr DlFRect kEmptyRect;

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_
