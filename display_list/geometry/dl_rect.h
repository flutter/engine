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

#define DL_ONLY_INT_FROM_FLOAT(Type) DL_ONLY_INT_FROM_FLOAT_M(FT, , Type)
#define DL_ONLY_INT_FROM_FLOAT_M(FT, Modifiers, Type) \
  template <typename IT = T, typename FT>             \
  Modifiers std::enable_if_t<                         \
      std::is_integral_v<IT> && std::is_floating_point_v<FT>, Type>

/// Templated struct for holding an axis-aligned rectangle.
///
/// Rectangles are defined as 4 axis-aligned edges that might contain
/// space. They can be viewed as 2 X coordinates that define the
/// left and right edges and 2 Y coordinates that define the top and
/// bottom edges; or they can be viewed as an origin and horizontal
/// and vertical dimensions (width and height).
///
/// When the left and right edges are reversed (right <= left) or the
/// top and bottom edges are reversed (bottom <= top), the rectangle
/// is considered empty. Viewing the rectangle in XYWH form, the width
/// and/or the height would be negative or zero, but the |width()| and
/// |height()| methods will return zero for simplicity. Such reversed/empty
/// rectangles contain no space and act as such in the methods that
/// operate on them (Intersection, Union, Intersects, Contains, etc.)
///
/// Rectangles cannot be modified by any method and a new value can only
/// be stored into an existing rect using assignment. This keeps the API
/// clean compared to implementations that might have similar methods
/// that produce the answer in place, or construct a new object with
/// the answer, or place the result in an indicated result object.
///
/// Methods that might fail to produce an answer will use |std::optional|
/// to indicate that success or failure (see |Intersection| and |CutOut|).
/// For convenience, |Intersection|, |CutOut| and |Union| all have overloaded
/// variants that take |std::optional| arguments and treat them as if
/// the argument was an empty rect to allow chaining multiple such methods
/// and only needing to check the optional condition of the final result.
/// These same methods also provide |...OrEmpty| overloaded variants that
/// translate an empty optional answer into a simple empty rectangle of the
/// same type.
///
/// Rounding methods which round a floating point rectangle in, out, or
/// to the nearest integer values are only defined on floating point
/// rectangles. In order to avoid confusing polymorphic return values
/// from such methods, the policy is that the methods that are called on
/// a floating point rectangle return the same floating point rectangle
/// type and the appropriately rounded values. Creating an integer rectangle
/// from a floating point rectangle will use similarly named "rounding
/// factory methods". For example:
/// - DlFRect.RoundedOut() returns a DlFRect (same as its object)
/// - but DlIRect::MakeRoundedOut(DlFRect) returns a DlIRect
///
/// NaN and Infinity values
///
/// Constructing an LTRB rectangle using Infinity values should work as
/// expected with either 0 or +Infinity returned as the size depending on
/// which side the Infinity values are on and the sign.
///
/// Constructing an XYWH rectangle using Infinity values will usually
/// not work if the math requires the object to compute a right or bottom
/// edge from ([xy] -Infinity + [wh] +Infinity). Other combinations might
/// work.
///
/// Any rectangle that is constructed with, or computed to have a NaN value
/// will be considered the same as any empty rectangle.
///
/// ---------------
/// Special notes on problems using the XYWH form of specifying rectangles:
///
/// It is possible to have integer rectangles whose dimensions exceed
/// the maximum number that their coordinates can represent since
/// (MAX_INT - MIN_INT) overflows the representable positive numbers.
/// Floating point rectangles technically have a similar issue in that
/// overflow can occur, but it will be automatically converted into
/// either an infinity, or a finite-overflow value and still be
/// representable, just with little to no precision.
///
/// Secondly, specifying such a rectangle leads to cases where the
/// math for (x+w) and/or (y+h) are also beyond the maximum representable
/// coordinates. For N-bit integer rectangles declared as XYWH, the
/// maximum right coordinate will require N+1 signed bits which cannot be
/// stored in storage that uses N-bit integers.
///
/// These problems are dealt with using the following implementation details:
///
/// Rectangles use an LTRB internal format which is the most convenient
/// for the majority of the work they must do.
///
/// Asking for the dimensions of a rectangle will clamp the answer to be
/// a non-negative number with all rectangles that are empty along that
/// dimension returning a 0 for an answer. Additionally, integer rectangles
/// such as |DlIRect| will return an unsigned dimension and clamp their
/// XYWH contructor parameters to the maximum signed value for right and
/// bottom coordinates.
///
/// Constructing an infinite floating point rectangle using LTRB will work
/// and will return infinity for the dimensions. Constructing such a
/// a rectangle using XYWH notation is not possible because
/// (-infinity + +infinity) has no answer (and produces NaN).
///
/// T: the type used to pass and store the coordinates
/// ST: the type used to pass and store dimensions (size)
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
    return distance <= static_cast<ST>(0) ? location : location + distance;
  }

 public:
  constexpr DlTRect() : DlTRect(zero_, zero_, zero_, zero_) {}

  constexpr DlTRect(const DlTRect& r) = default;
  constexpr DlTRect(DlTRect&& r) = default;

  /// Produces a rectangle from 4 coordinate values in the LTRB style
  static constexpr DlTRect MakeLTRB(T left, T top, T right, T bottom) {
    return {left, top, right, bottom};
  }

  /// Produces a rectangle from 2 coordinate values and 2 dimension (size)
  /// values in the XYWH style.
  /// Overflow of the origin plus the dimensions is handled by clamping
  /// to the maximum representable value (even if that is infinity for
  /// floating point types).
  /// Underflow of the origin plus a negative dimension is avoided by
  /// clamping the minimum dimension to 0
  static constexpr DlTRect MakeXYWH(T x, T y, ST w, ST h) {
    return {x, y, SafeAdd(x, w), SafeAdd(y, h)};
  }

  /// Produces a rectangle whose origin is (0, 0) and with the indicated
  /// dimensions.
  static constexpr DlTRect MakeWH(T w, T h) {
    return MakeLTRB(zero_, zero_, w, h);
  }

  /// Produces a rectangle from the indicated dimensional object that
  /// implements both |width()| and |height()| methods that return the
  /// same type as used for the coordinates of this template.
  template <typename U>
  static constexpr DlTRect MakeSize(DlTSize<U> size) {
    return MakeLTRB(zero_, zero_, size.width(), size.height());
  }

  /// Produces a rectangle from a similar DlTPoint object and a similar
  /// DlTSize object which are templated from the coordinate and dimension
  /// types of this template.
  static constexpr DlTRect MakeOriginSize(DlTPoint<T> origin,
                                          DlTSize<ST> size) {
    return MakeXYWH(origin.x(), origin.y(), size.width(), size.height());
  }

  /// Produces a rectangle from the indicated bouded object that
  /// implements all of |left()|, |top()|, |right()|, and |bottom()|
  /// methods that return the same type as used for the coordinates
  /// of this template.
  template <typename U>
  static constexpr DlTRect MakeBounds(const U& r) {
    return MakeLTRB(r.left(), r.top(), r.right(), r.bottom());
  }

  /// Produces a(n integer-templates-only) rectangle from the indicated
  /// (float-templates-only) rectangle by rounding its coordinates out
  /// to the smallest rectangle that contains all of the points of the
  /// original.
  DL_ONLY_INT_FROM_FLOAT_M(FT, static constexpr, DlTRect)
  MakeRoundedOut(const DlTRect<FT, FT>& r) {
    return MakeLTRB(                      //
        DlScalar_ToInt(floor(r.left())),  //
        DlScalar_ToInt(floor(r.top())),   //
        DlScalar_ToInt(ceil(r.right())),  //
        DlScalar_ToInt(ceil(r.bottom()))  //
    );
  }

  /// Produces a(n integer-templates-only) rectangle from the indicated
  /// (float-templates-only) rectangle by rounding its coordinates in
  /// to the largest rectangle that contains all of the integer points
  /// of the original.
  DL_ONLY_INT_FROM_FLOAT_M(FT, static constexpr, DlTRect)
  MakeRoundedIn(const DlTRect<FT, FT>& r) {
    return MakeLTRB(                       //
        DlScalar_ToInt(ceil(r.left())),    //
        DlScalar_ToInt(ceil(r.top())),     //
        DlScalar_ToInt(floor(r.right())),  //
        DlScalar_ToInt(floor(r.bottom()))  //
    );
  }

  /// Produces a(n integer-templates-only) rectangle from the indicated
  /// (float-templates-only) rectangle by rounding its coordinates to
  /// the nearest rectangle that contains all of the same "pixels" that
  /// were wholly contained in the original and some of the border
  /// "pixels" as well as defined by the actions of the |round()| method.
  DL_ONLY_INT_FROM_FLOAT_M(FT, static constexpr, DlTRect)
  MakeRounded(const DlTRect<FT, FT>& r) {
    return MakeLTRB(                       //
        DlScalar_ToInt(round(r.left())),   //
        DlScalar_ToInt(round(r.top())),    //
        DlScalar_ToInt(round(r.right())),  //
        DlScalar_ToInt(round(r.bottom()))  //
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
  /// Returns the non-negative width of the rectangle with all empty
  /// rectangles producing a zero width.
  constexpr inline ST width() const { return DL_RECT_DIM(left_, right_); }
  /// Returns the non-negative height of the rectangle with all empty
  /// rectangles producing a zero height.
  constexpr inline ST height() const { return DL_RECT_DIM(top_, bottom_); }
  constexpr inline DlTSize<ST> size() const {
    return DlTSize<ST>(width(), height());
  }
#undef DL_RECT_DIM

  /// Returns true if the rectangle contains no area or contains NaN values.
  constexpr inline bool IsEmpty() const {
    return !(right_ > left_ && bottom_ > top_);
  }

  DL_ONLY_ON_FLOAT(bool)
  /// For rectanles with floating point coordinates, indicates if all of
  /// its values are finite (and non-NaN).
  constexpr IsFinite() const { return DlScalars_AreAllFinite(&left_, 4); }

  /// Returns the 4 corners of the rectangle in the clockwise order:
  /// [UL, UR, LR, LL]
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

  /// Returns a rectangle with its left and right edges and its top
  /// and bottom edges sorted so as to be non-empty as long as neither
  /// resulting dimension is 0.
  /// REMIND: since MakeXYWH already clamps the dimensions to be
  /// non-zero, and since this method produces a new object to hold
  /// its result, perhaps it should be represented as a factory
  /// method instead. It is rarely used.
  [[nodiscard]] constexpr DlTRect Sorted() const {
    T l, r, t, b;
    DL_SORT(l, r, left_, right_);
    DL_SORT(t, b, top_, bottom_);
    return MakeLTRB(l, t, r, b);
  }
#undef DL_SORT

#define DL_ROUNDED_UL_LR(ULF, LRF) \
  MakeLTRB((ULF)(left_), (ULF)(top_), (LRF)(right_), (LRF)(bottom_))

  /// Only defined on floating point rectangles.
  /// Returns a new rectangle that contains all of the points of this
  /// rectangle by rounding all edges out to the next integer coordinate.
  DL_ONLY_ON_FLOAT_M([[nodiscard]] constexpr, DlTRect)
  RoundedOut() const { return DL_ROUNDED_UL_LR(floor, ceil); }

  /// Only defined on floating point rectangles.
  /// Returns a new rectangle that contains all of the integer points within
  /// this rectangle by rounding all edges in to the next integer coordinate.
  DL_ONLY_ON_FLOAT_M([[nodiscard]] constexpr, DlTRect)
  RoundedIn() const { return DL_ROUNDED_UL_LR(ceil, floor); }

  /// Returns a new rectangle that contains all of the "pixels" wholly
  /// contained within and some of the border "pixels" as well as defined
  /// by the actions of the |round()| method.
  DL_ONLY_ON_FLOAT_M([[nodiscard]] constexpr, DlTRect)
  Rounded() const { return DL_ROUNDED_UL_LR(round, round); }

#undef DL_ROUNDED_UL_LR

  /// Returns a point representing the coordinate closes to the center
  /// of this rectangle with rounding appropriate to the equation (a+b)/2
  /// in the coordinate format of this rectangle.
  [[nodiscard]] DlTPoint<T> Center() const {
    return DlTPoint<T>((left_ + right_) / static_cast<T>(2),
                       (top_ + bottom_) / static_cast<T>(2));
  }

  /// Returns a rectangle defined by this rectangle's upper left corner
  /// and its |Center()|.
  [[nodiscard]] DlTRect UpperLeftQuadrant() const {
    auto center = Center();
    return {left_, top_, center.x(), center.y()};
  }

  /// Returns a rectangle defined by this rectangle's upper right corner
  /// and its |Center()|.
  [[nodiscard]] DlTRect UpperRightQuadrant() const {
    auto center = Center();
    return {center.x(), top_, right_, center.y()};
  }

  /// Returns a rectangle defined by this rectangle's lower right corner
  /// and its |Center()|.
  [[nodiscard]] DlTRect LowerRightQuadrant() const {
    auto center = Center();
    return {center.x(), center.y(), right_, bottom_};
  }

  /// Returns a rectangle defined by this rectangle's lower left corner
  /// and its |Center()|.
  [[nodiscard]] DlTRect LowerLeftQuadrant() const {
    auto center = Center();
    return {left_, center.y(), center.x(), bottom_};
  }

  /// Returns a rectangle padded out by the indicated margin values with
  /// positive values expanding the rectangle and negative values
  /// contracting it.
  [[nodiscard]] DlTRect Expand(T marginX, T marginY) const {
    return {
        left_ - marginX,
        top_ - marginY,
        right_ + marginX,
        bottom_ + marginY,
    };
  }
  [[nodiscard]] DlTRect Expand(DlTPoint<T> v) const {
    return Expand(v.x(), v.y());
  }

  /// Returns a rectangle translated by the indicated offset values.
  [[nodiscard]] DlTRect Translate(T dx, T dy) const {
    return {
        left_ + dx,
        top_ + dy,
        right_ + dx,
        bottom_ + dy,
    };
  }
  [[nodiscard]] DlTRect Translate(DlTPoint<T> v) const {
    return Translate(v.x(), v.y());
  }

  /// Returns the geometric union of this rectangle and the specified
  /// rectangle.
  DlTRect Union(const DlTRect& r) const {
    if (r.IsEmpty()) {
      return *this;
    }
    if (IsEmpty()) {
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

  /// Returns the geometric intersection of this rectangle and the specified
  /// rectangle, or |std::nullopt| if there is no intersection.
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

  /// Returns a rectangle produced from subtracting the area of the specified
  /// rectangle and returning the bounds of that result or |std::nullopt| if
  /// the result is empty.
  std::optional<DlTRect> CutOut(const DlTRect& sub) const {
    T src_left = this->left_;
    T src_right = this->right_;
    T src_top = this->top_;
    T src_bottom = this->bottom_;
    if (!(src_right > src_left && src_bottom > src_top)) {
      // src has NaN values or is empty, result is empty
      // Return nullopt to normalize all empty results
      return std::nullopt;
    }
    if (sub.left_ <= src_left && sub.right_ >= src_right) {
      // horizontally, sub is non-empty and has no NaN values
      // sub spans the entire width of this (source) rect so
      // we can slice off a top or bottom edge of the src for
      // the result.
      if (!(src_bottom > sub.top_ && sub.bottom_ > src_top)) {
        // vertically, sub has NaN values or no intersection,
        // no change in the rect
        return *this;
      }
      if (sub.top_ <= src_top) {
        src_top = sub.bottom_;
      }
      if (sub.bottom_ >= src_bottom) {
        src_bottom = sub.top_;
      }
      if (src_top < src_bottom) {
        return MakeLTRB(src_left, src_top, src_right, src_bottom);
      } else {
        return std::nullopt;
      }
    } else if (sub.top_ <= src_top && sub.bottom_ >= src_bottom) {
      // vertically, sub is non-empty and has no NaN values
      // sub spans the entire height of this (source) rect so
      // we can slice off a left or right edge of the src for
      // the result.
      if (!(src_right > sub.left_ && sub.right_ > src_left)) {
        // horizontally, sub has NaN values or no intersection,
        // no change in the rect
        return *this;
      }
      if (sub.left_ <= src_left) {
        src_left = sub.right_;
      }
      if (sub.right_ >= src_right) {
        src_right = sub.left_;
      }
      if (src_left < src_right) {
        return MakeLTRB(src_left, src_top, src_right, src_bottom);
      } else {
        return std::nullopt;
      }
    }
    return *this;
  }
  std::optional<DlTRect> CutOut(const std::optional<DlTRect>& sub) const {
    return sub.has_value() ? CutOut(sub.value()) : *this;
  }
  DlTRect CutOutOrEmpty(const DlTRect& sub) const {
    return CutOut(sub).value_or(DlTRect());
  }
  DlTRect CutOutOrEmpty(const std::optional<DlTRect>& sub) const {
    return CutOut(sub).value_or(DlTRect());
  }

  /// Returns true iff the indicated point is inside this rectangle using
  /// a half-empty determination that prevents points from being considered
  /// inside 2 adjacent rectangles. Points along the top and left edges
  /// if this rectangle are considered inside it (if it is not empty) and
  /// points along the right and bottom edges are considered outside.
  bool Contains(T x, T y) const {
    return !this->IsEmpty() && DlScalars_AreFinite(x, y) &&  //
           x >= left_ && x < right_ &&                       //
           y >= top_ && y < bottom_;
  }
  bool Contains(DlTPoint<T> p) const { return Contains(p.x(), p.y()); }

  /// Returns true iff there exists a point that is considered inside the
  /// indicated rectangle and also considered inside this rectangle, as
  /// determined by the |Contains(x, y)| method.
  bool Intersects(const DlTRect& r) const {
    return !this->IsEmpty() && !r.IsEmpty() &&  //
           this->left_ < r.right_ && r.left_ < this->right_ &&
           this->top_ < r.bottom_ && r.top_ < this->bottom_;
  }

  /// Returns true iff every point considered inside the indicated
  /// rectangle is also considered inside this rectangle, as determined
  /// by the |Contains(x, y)| method.
  bool Contains(const DlTRect& r) const {
    return !this->IsEmpty() && !r.IsEmpty() &&  //
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
