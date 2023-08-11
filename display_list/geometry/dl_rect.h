// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_

#include <algorithm>
#include <limits>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_size.h"

namespace flutter {

#define DL_ONLY_ON_INT(Type) DL_ONLY_ON_INT_M(, Type)
#define DL_ONLY_ON_INT_M(Modifiers, Type) \
  template <typename U = T>         \
  Modifiers                         \
  std::enable_if_t<std::is_integral_v<U>, Type>

#define DL_ONLY_ON_FLOAT(Type) DL_ONLY_ON_FLOAT_M(, Type)
#define DL_ONLY_ON_FLOAT_M(Modifiers, Type) \
  template <typename U = T>           \
  Modifiers                           \
  std::enable_if_t<std::is_floating_point_v<U>, Type>

#define DL_ONLY_FROM_FLOAT(FT, Type) DL_ONLY_FROM_FLOAT_M(FT, , Type)
#define DL_ONLY_FROM_FLOAT_M(FT, Modifiers, Type) \
  template <typename FT>                      \
  Modifiers                                   \
  std::enable_if_t<std::is_floating_point_v<FT>, Type>

#define DL_ONLY_INT_FROM_FLOAT(Type) DL_ONLY_INT_FROM_FLOAT_M(IT, FT, Type)
#define DL_ONLY_INT_FROM_FLOAT_M(IT, FT, Modifiers, Type)   \
  template <typename IT = T, typename FT>        \
  Modifiers                                      \
  std::enable_if_t<std::is_integral_v<IT> &&     \
                   std::is_floating_point_v<FT>, \
                   Type>

template <typename T, typename LT>
struct DlTRect {
 protected:
  static constexpr T zero_ = static_cast<T>(0);

  T left_;
  T top_;
  T right_;
  T bottom_;

  constexpr DlTRect(T left, T top, T right, T bottom)
      : left_(left), top_(top), right_(right), bottom_(bottom) {}

  template <typename = std::enable_if<std::is_integral_v<T>>>
  static T SafeAdd(T location, LT distance) {
    if (distance <= static_cast<LT>(0)) {
      return location;
    }
    T result = location + distance;
    if (result < location) {
      return location;
    }
    return result;
  }

  static inline T SafeAdd(T location, LT distance) {
    return location + distance;
  }

 public:
  constexpr DlTRect() : DlTRect(zero_, zero_, zero_, zero_) {}

  constexpr DlTRect(const DlTRect& r) = default;
  constexpr DlTRect(DlTRect&& r) = default;

  DlTRect& operator=(const DlTRect& r) = default;
  DlTRect& operator=(DlTRect&& r) = default;

  constexpr inline T left() const { return left_; }
  constexpr inline T top() const { return top_; }
  constexpr inline T right() const { return right_; }
  constexpr inline T bottom() const { return bottom_; }

  inline void SetLeft(T left) { left_ = left; }
  inline void SetTop(T top) { top_ = top; }
  inline void SetRight(T right) { right_ = right; }
  inline void SetBottom(T bottom) { bottom_ = bottom; }

  inline void SetLTRB(T left, T top, T right, T bottom) {
    left_ = left;
    top_ = top;
    right_ = right;
    bottom_ = bottom;
  }

  inline void SetXYWH(T x, T y, LT w, LT h) {
    left_ = x;
    top_ = y;
    right_ = x + w;
    bottom_ = y + h;
  }

  DL_ONLY_FROM_FLOAT_M(FT, inline, void)
  SetRoundedOut(const DlTRect<FT, FT>& r) {
    left_ = static_cast<T>(floor(r.left()));
    top_ = static_cast<T>(floor(r.top()));
    right_ = static_cast<T>(ceil(r.right()));
    bottom_ = static_cast<T>(ceil(r.bottom()));
  }

  DL_ONLY_FROM_FLOAT_M(FT, inline, void)
  SetRoundedIn(const DlTRect<FT, FT>& r) {
    left_ = static_cast<T>(ceil(r.left()));
    top_ = static_cast<T>(ceil(r.top()));
    right_ = static_cast<T>(floor(r.right()));
    bottom_ = static_cast<T>(floor(r.bottom()));
  }

  DL_ONLY_FROM_FLOAT_M(FT, inline, void)
  SetRounded(const DlTRect<FT, FT>& r) {
    left_ = static_cast<T>(round(r.left()));
    top_ = static_cast<T>(round(r.top()));
    right_ = static_cast<T>(round(r.right()));
    bottom_ = static_cast<T>(round(r.bottom()));
  }

  template <typename U>
  inline void Set(const U& r) {
    left_ = r.left();
    top_ = r.top();
    right_ = r.right();
    bottom_ = r.bottom();
  }

  bool operator==(const DlTRect& r) const {
    return left_ == r.left_ &&    //
           top_ == r.top_ &&      //
           right_ == r.right_ &&  //
           bottom_ == r.bottom_;
  }
  bool operator!=(const DlTRect& r) const { return !(*this == r); }

  static constexpr DlTRect MakeLTRB(T left, T top, T right, T bottom) {
    return {left, top, right, bottom};
  }

  static constexpr DlTRect MakeXYWH(T x, T y, LT w, LT h) {
    return {x, y, SafeAdd(x, w), SafeAdd(y, h)};
  }

  static constexpr DlTRect MakeWH(T w, T h) {
    return {zero_, zero_, w, h};
  }

  template <typename U>
  static constexpr DlTRect MakeSize(DlTSize<U> size) {
    return MakeXYWH(0, 0, size.width(), size.height());
  }

  static constexpr DlTRect MakeOriginSize(DlTPoint<T> origin,
                                          DlTSize<LT> size) {
    return MakeXYWH(origin.x(), origin.y(), size.width(), size.height());
  }

  template <typename U>
  static constexpr DlTRect MakeBounds(const U& r) {
    return MakeLTRB(r.left(), r.top(), r.right(), r.bottom());
  }

  DL_ONLY_FROM_FLOAT_M(FT, static constexpr, DlTRect)
  MakeRoundedOut(const DlTRect<FT, FT>& r) {
    return MakeLTRB(
      static_cast<T>(floor(r.left())),
      static_cast<T>(floor(r.top())),
      static_cast<T>(ceil(r.right())),
      static_cast<T>(ceil(r.bottom()))
    );
  }

  DL_ONLY_FROM_FLOAT_M(FT, static constexpr, DlTRect)
  MakeRoundedIn(const DlTRect<FT, FT>& r) {
    return MakeLTRB(
      static_cast<T>(ceil(r.left())),
      static_cast<T>(ceil(r.top())),
      static_cast<T>(floor(r.right())),
      static_cast<T>(floor(r.bottom()))
    );
  }

  DL_ONLY_FROM_FLOAT_M(FT, static constexpr, DlTRect)
  MakeRounded(const DlTRect<FT, FT>& r) {
    return MakeLTRB(
      static_cast<T>(round(r.left())),
      static_cast<T>(round(r.top())),
      static_cast<T>(round(r.right())),
      static_cast<T>(round(r.bottom()))
    );
  }

#define DL_SORT(OUTA, OUTB, INA, INB) \
  do {                                \
    if (INA <= INB) {                 \
      OUTA = INA;                     \
      OUTB = INB;                     \
    } else {                          \
      OUTA = INB;                     \
      OUTB = INA;                     \
    }                                 \
  } while(0)

  constexpr DlTRect MakeSorted() const {
    T l, r, t, b;
    DL_SORT(l, r, left_, right_);
    DL_SORT(t, b, top_, bottom_);
    return MakeLTRB(l, t, r, b);
  }

  // Returns the 4 corners of the rectangle in the order UL, UR, LR, LL
  void ToQuad(DlTPoint<T> quad[4]) const {
    quad[0] = {left_, top_};
    quad[1] = {right_, top_};
    quad[2] = {right_, bottom_};
    quad[3] = {left_, bottom_};
  }

  void SetEmpty() {
    left_ = top_ = right_ = bottom_ = zero_;
  };

  T x() const { return left_; }
  T y() const { return top_; }
  DlTPoint<T> origin() const { return DlTPoint<T>(left_, top_); }

  LT width() const { return left_ <= right_ ? right_ - left_ : 0; }
  LT height() const { return top_ <= bottom_ ? bottom_ - top_ : 0; }
  DlTSize<LT> size() const { return DlTSize<LT>(width(), height()); }

  bool is_empty() const { return !(right_ > left_ && bottom_ > top_); }

  DL_ONLY_ON_FLOAT(bool)
  is_finite() const {
    return DlScalars_AreAllFinite(&left_, 4);
  }

  DL_ONLY_ON_FLOAT(void)
  RoundOut() {
    left_ = floor(left_),
    top_ = floor(top_),
    right_ = ceil(right_),
    bottom_ = ceil(bottom_);
  }

  DL_ONLY_ON_FLOAT(void)
  RoundIn() {
    left_ = ceil(left_),
    top_ = ceil(top_),
    right_ = floor(right_),
    bottom_ = floor(bottom_);
  }

  DL_ONLY_ON_FLOAT(void)
  Round() {
    left_ = round(left_),
    top_ = round(top_),
    right_ = round(right_),
    bottom_ = round(bottom_);
  }

  DlTPoint<T> Center() const {
    return DlTPoint<T>((left_ + right_) / static_cast<T>(2),
                       (top_ + bottom_) / static_cast<T>(2));
  }

  DlTRect UpperLeftQuadrant() const {
    auto center = Center();
    return {left_, top_, center.x(), center.y()};
  }

  DlTRect UpperRightQuadrant() const {
    auto center = Center();
    return {center.x(), top_, right_, center.y()};
  }

  DlTRect LowerLeftQuadrant() const {
    auto center = Center();
    return {left_, center.y(), center.x(), bottom_};
  }

  DlTRect LowerRightQuadrant() const {
    auto center = Center();
    return {center.x(), center.y(), right_, bottom_};
  }

  void Inset(T marginX, T marginY) {
    left_ += marginX;
    top_ += marginY;
    right_ -= marginX;
    bottom_ -= marginY;
  }
  void Inset(DlTPoint<T> v) { Inset(v.x(), v.y()); }
  DlTRect MakeInset(T marginX, T marginY) const {
    return {left_ + marginX, top_ + marginY, right_ - marginX, bottom_ - marginY };
  }
  DlTRect MakeInset(DlTPoint<T> v) const { return MakeInset(v.x(), v.y()); }

  void Outset(T marginX, T marginY) {
    left_ -= marginX;
    top_ -= marginY;
    right_ += marginX;
    bottom_ += marginY;
  }
  void Outset(DlTPoint<T> v) { Outset(v.x(), v.y()); }
  DlTRect MakeOutset(T marginX, T marginY) const {
    return {left_ - marginX, top_ - marginY, right_ + marginX, bottom_ + marginY };
  }
  DlTRect MakeOutset(DlTPoint<T> v) const { return MakeOutset(v.x(), v.y()); }

  void Offset(T dx, T dy) {
    left_ += dx;
    top_ += dy;
    right_ += dx;
    bottom_ += dy;
  }
  void Offset(DlTPoint<T> v) { Offset(v.x(), v.y()); }
  DlTRect MakeOffset(T dx, T dy) const {
    return {left_ + dx, top_ + dy, right_ + dx, bottom_ + dy};
  }
  DlTRect MakeOffset(DlTPoint<T> v) const { return MakeOffset(v.x(), v.y()); }

  void Join(const DlTRect* r) {
    if (!r->is_empty()) {
      if (is_empty()) {
        *this = *r;
      } else {
        left_ = std::min(left_, r->left_);
        top_ = std::min(top_, r->top_);
        right_ = std::max(right_, r->right_);
        bottom_ = std::max(bottom_, r->bottom_);
      }
    }
  }
  void Join(const DlTRect& r) { Join(&r); }

  bool Intersect(const DlTRect* r) {
    if (is_empty()) {
      return false;
    }
    if (r->is_empty()) {
      SetEmpty();
      return false;
    }
    left_ = std::max(left_, r->left_);
    top_ = std::max(top_, r->top_);
    right_ = std::min(right_, r->right_);
    bottom_ = std::min(bottom_, r->bottom_);
    return !is_empty();
  }
  bool Intersect(const DlTRect& r) { return Intersect(&r); }

  bool Intersects(const DlTRect* r) const {
    return !this->is_empty() &&
           this->left_ < r->right_ && r->left_ < this->right_ &&
           this->top_ < r->bottom_ && r->top_ < this->bottom_;
  }
  bool Intersects(const DlTRect& r) const { return Intersects(&r); }

  bool Contains(T x, T y) {
    return !this->is_empty() && DlScalars_AreFinite(x, y) &&  //
           x >= left_ && x < right_ &&                        //
           y >= top_ && y < bottom_;
  }
  bool Contains(DlTPoint<T> p) { return Contains(p.x(), p.y()); }

  bool Contains(const DlTRect* r) const {
    return !this->is_empty() && !r->is_empty() &&
           this->left_ <= r->left_ && this->right_ >= r->right_ &&
           this->top_ <= r->top_ && this->bottom_ >= r->bottom_;
  }
  bool Contains(const DlTRect& r) const { return Contains(&r); }
};

#undef DL_ONLY_ON_INT
#undef DL_ONLY_ON_INT_M
#undef DL_ONLY_ON_FLOAT
#undef DL_ONLY_ON_FLOAT_M
#undef DL_ONLY_FROM_FLOAT
#undef DL_ONLY_FROM_FLOAT_M
#undef DL_ONLY_INT_FROM_FLOAT
#undef DL_ONLY_INT_FROM_FLOAT_M

using DlIRect = DlTRect<DlInt, DlSize>;
using DlFRect = DlTRect<DlScalar, DlScalar>;

template <typename T, typename LT>
inline std::ostream& operator<<(std::ostream& os, const DlTRect<T,LT>& rect) {
  return os << "DlRect(" << rect.left() << ", " << rect.top() << " => "
                         << rect.right() << ", " << rect.bottom() << ")";
}

[[maybe_unused]]
constexpr DlFRect kMaxCullRect = DlFRect::MakeLTRB(-1E9F, -1E9F, 1E9F, 1E9F);

[[maybe_unused]]
constexpr DlFRect kEmptyRect;

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_RECT_H_
