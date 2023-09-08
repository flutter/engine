// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_DISPLAY_LIST_GEOMETRY_DL_PATH_H_
#define FLUTTER_DISPLAY_LIST_GEOMETRY_DL_PATH_H_

#include <vector>

#include "flutter/display_list/dl_base_types.h"
#include "flutter/display_list/geometry/dl_angle.h"
#include "flutter/display_list/geometry/dl_point.h"
#include "flutter/display_list/geometry/dl_rect.h"
#include "flutter/display_list/geometry/dl_round_rect.h"
#include "flutter/display_list/geometry/dl_transform.h"

#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkRRect.h"

namespace flutter {

class DlPath {
 public:
  enum class FillType {
    // Specifies that "inside" is computed by a non-zero sum of
    // signed edge crossings
    kWinding,
    // Specifies that "inside" is computed by an odd number of
    // edge crossings
    kEvenOdd,
    // Same as |kWinding|, but draws outside of the path, rather than inside
    kInverseWinding,
    // Same as |kEvenOdd|, but draws outside of the path, rather than inside
    kInverseEvenOdd
  };

  enum class Direction {
    kCW,   // clockwise direction for adding closed contours
    kCCW,  // counter-clockwise direction for adding closed contours
  };

  // For arcs defined by an oval with radii (rx, ry) starting at the
  // current point and ending at (x, y), four different parts of that
  // oval might intercept those 2 coordinates.  ArcSize and Direction
  // combine to allow the caller to choose from one of the four parts
  // of the oval which might match the given coordinate constraints.
  enum class ArcSize {
    kSmall_ArcSize,  // smaller of the matching arc pairs
    kLarge_ArcSize,  // larger of the matching arc pairs
  };

  enum class PathOp {
    kDifference,         // subtract the op path from the first path
    kIntersect,          // intersect the two paths
    kUnion,              // union (inclusive-or) the two paths
    kXOR,                // exclusive-or the two paths
    kReverseDifference,  // subtract the first path from the op path
  };

  enum class Verb {
    kMoveTo,
    kLineTo,
    kQuadTo,
    kConicTo,
    kCubicTo,
  };

  DlPath() {}
  DlPath(const DlPath& path) : path_(path.path_) {}
  DlPath(DlPath&& path) : path_(path.path_) {}
  DlPath(const SkPath& sk_path) : path_(sk_path) {}

  DlPath& operator=(const DlPath& path) = default;
  DlPath& operator=(DlPath&& path) = default;

  static DlPath MakeLine(DlScalar x1, DlScalar y1, DlScalar x2, DlScalar y2) {
    return DlPath().MoveTo(x1, y1).LineTo(x2, y2);
  }
  static DlPath MakeLine(DlFPoint p1, DlFPoint p2) {
    return MakeLine(p1.x(), p1.y(), p2.x(), p2.y());
  }

  static DlPath MakeRectLTRB(DlScalar left,
                             DlScalar top,
                             DlScalar right,
                             DlScalar bottom) {
    return DlPath().AddRectLTRB(left, top, right, bottom);
  }
  static DlPath MakeRectXYWH(DlScalar x,
                             DlScalar y,
                             DlScalar width,
                             DlScalar height) {
    return DlPath().AddRectXYWH(x, y, width, height);
  }
  static DlPath MakeRect(const DlFRect& rect) { return DlPath().AddRect(rect); }

  static DlPath MakeRoundRect(const DlFRect& rect, DlScalar dx, DlScalar dy) {
    return DlPath().AddRoundRect(rect, dx, dy);
  }

  static DlPath MakeOvalLTRB(DlScalar left,
                             DlScalar top,
                             DlScalar right,
                             DlScalar bottom) {
    return DlPath().AddOvalLTRB(left, top, right, bottom);
  }
  static DlPath MakeOvalXYWH(DlScalar x,
                             DlScalar y,
                             DlScalar width,
                             DlScalar height) {
    return DlPath().AddOvalXYWH(x, y, width, height);
  }
  static DlPath MakeOval(const DlFRect& rect) { return DlPath().AddOval(rect); }

  static DlPath MakeCircle(DlScalar center_x,
                           DlScalar center_y,
                           DlScalar radius) {
    return DlPath().AddCircle(center_x, center_y, radius);
  }

  static DlPath MakePolygon(const DlFPoint vertices[],
                            int count,
                            bool is_closed) {
    DlPath path;
    if (count > 0) {
      path.MoveTo(vertices[0]);
      for (int i = 1; i < count; i++) {
        path.LineTo(vertices[i]);
      }
      if (is_closed) {
        path.Close();
      }
    }
    return path;
  }
  static DlPath MakePolygon(const std::initializer_list<DlFPoint>& list,
                            bool isClosed) {
    return MakePolygon(list.begin(), SkToInt(list.size()), isClosed);
  }

  FillType fill_type() const {
    return static_cast<FillType>(path_.getFillType());
  }

  int verb_count() const { return path_.countVerbs(); }
  int point_count() const { return path_.countPoints(); }

  bool operator==(const DlPath& path) const { return path_ == path.path_; }
  bool operator!=(const DlPath& path) const { return !(*this == path); }

  bool is_valid() const { return path_.isValid(); }

  bool is_volatile() const { return path_.isVolatile(); }

  bool is_inverse_fill_type() const { return path_.isInverseFillType(); }

  bool is_rect(DlFRect* rect) const {
    return path_.isRect(reinterpret_cast<SkRect*>(rect));
  }

  bool is_oval(DlFRect* rect) const {
    return path_.isOval(reinterpret_cast<SkRect*>(rect));
  }

  bool is_rrect(DlFRRect* r_rect) const {
    SkRRect sk_r_rect;
    if (!path_.isRRect(&sk_r_rect)) {
      return false;
    }
    if (r_rect) {
      DlFVector radii[4];
      radii[0] = sk_r_rect.radii(SkRRect::kUpperLeft_Corner);
      radii[1] = sk_r_rect.radii(SkRRect::kUpperRight_Corner);
      radii[2] = sk_r_rect.radii(SkRRect::kLowerRight_Corner);
      radii[3] = sk_r_rect.radii(SkRRect::kLowerLeft_Corner);
      *r_rect = DlFRRect::MakeRectRadii(
          *reinterpret_cast<const DlFRect*>(&sk_r_rect.rect()), radii);
    }
    return true;
  }

  DlPath& SetIsVolatile(bool is_volatile) {
    path_.setIsVolatile(is_volatile);
    return *this;
  }

  void SetFillType(FillType type) {
    path_.setFillType(static_cast<SkPathFillType>(type));
  }

  DlPath& MoveTo(DlScalar x, DlScalar y) {
    path_.moveTo(x, y);
    return *this;
  }
  DlPath& MoveTo(DlFPoint p) { return MoveTo(p.x(), p.y()); }

  DlPath& RelativeMoveTo(DlScalar x, DlScalar y) {
    path_.rMoveTo(x, y);
    return *this;
  }
  DlPath& RelativeMoveTo(DlFPoint p) { return RelativeMoveTo(p.x(), p.y()); }

  DlPath& LineTo(DlScalar x, DlScalar y) {
    path_.lineTo(x, y);
    return *this;
  }
  DlPath& LineTo(DlFPoint p) { return LineTo(p.x(), p.y()); }

  DlPath& RelativeLineTo(DlScalar x, DlScalar y) {
    path_.rLineTo(x, y);
    return *this;
  }
  DlPath& RelativeLineTo(DlFPoint p) { return RelativeLineTo(p.x(), p.y()); }

  DlPath& QuadTo(DlScalar x1, DlScalar y1, DlScalar x2, DlScalar y2) {
    path_.quadTo(x1, y1, x2, y2);
    return *this;
  }
  DlPath& QuadTo(DlFPoint p1, DlFPoint p2) {
    return QuadTo(p1.x(), p1.y(), p2.x(), p2.y());
  }

  DlPath& RelativeQuadTo(DlScalar x1, DlScalar y1, DlScalar x2, DlScalar y2) {
    path_.rQuadTo(x1, y1, x2, y2);
    return *this;
  }
  DlPath& RelativeQuadTo(DlFPoint p1, DlFPoint p2) {
    return RelativeQuadTo(p1.x(), p1.y(), p2.x(), p2.y());
  }

  DlPath& ConicTo(DlScalar x1,
                  DlScalar y1,
                  DlScalar x2,
                  DlScalar y2,
                  DlScalar weight) {
    path_.conicTo(x1, y1, x2, y2, weight);
    return *this;
  }
  DlPath& ConicTo(DlFPoint p1, DlFPoint p2, DlScalar weight) {
    return ConicTo(p1.x(), p1.y(), p2.x(), p2.y(), weight);
  }

  DlPath& RelativeConicTo(DlScalar x1,
                          DlScalar y1,
                          DlScalar x2,
                          DlScalar y2,
                          DlScalar weight) {
    path_.rConicTo(x1, y1, x2, y2, weight);
    return *this;
  }
  DlPath& RelativeConicTo(DlFPoint p1, DlFPoint p2, DlScalar weight) {
    return RelativeConicTo(p1.x(), p1.y(), p2.x(), p2.y(), weight);
  }

  DlPath& CubicTo(DlScalar x1,
                  DlScalar y1,
                  DlScalar x2,
                  DlScalar y2,
                  DlScalar x3,
                  DlScalar y3) {
    path_.cubicTo(x1, y1, x2, y2, x3, y3);
    return *this;
  }
  DlPath& CubicTo(DlFPoint p1, DlFPoint p2, DlFPoint p3) {
    return CubicTo(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
  }

  DlPath& RelativeCubicTo(DlScalar x1,
                          DlScalar y1,
                          DlScalar x2,
                          DlScalar y2,
                          DlScalar x3,
                          DlScalar y3) {
    path_.rCubicTo(x1, y1, x2, y2, x3, y3);
    return *this;
  }
  DlPath& RelativeCubicTo(DlFPoint p1, DlFPoint p2, DlFPoint p3) {
    return RelativeCubicTo(p1.x(), p1.y(), p2.x(), p2.y(), p3.x(), p3.y());
  }

  void ArcTo(DlFRect oval,
             DlAngle startAngle,
             DlAngle sweepAngle,
             bool forceMoveTo) {
    path_.arcTo(*reinterpret_cast<const SkRect*>(&oval),  //
                startAngle.degrees(), sweepAngle.degrees(), forceMoveTo);
  }

  void ArcToPoint(DlScalar radius_x,
                  DlScalar radius_y,
                  DlAngle x_axis_rotation,
                  ArcSize arc_size,
                  Direction dir,
                  DlScalar arc_end_x,
                  DlScalar arc_end_y) {
    path_.arcTo(radius_x, radius_y, x_axis_rotation.degrees(),
                static_cast<SkPath::ArcSize>(arc_size),
                static_cast<SkPathDirection>(dir), arc_end_x, arc_end_y);
  }

  void RelativeArcToPoint(DlScalar radius_x,
                          DlScalar radius_y,
                          DlAngle x_axis_rotation,
                          ArcSize arc_size,
                          Direction dir,
                          DlScalar arc_end_x,
                          DlScalar arc_end_y) {
    path_.rArcTo(radius_x, radius_y, x_axis_rotation.degrees(),
                 static_cast<SkPath::ArcSize>(arc_size),
                 static_cast<SkPathDirection>(dir), arc_end_x, arc_end_y);
  }

  DlPath& Close() {
    path_.close();
    return *this;
  }

  DlPath& Reset() {
    path_.reset();
    return *this;
  }

  void Offset(DlScalar dx, DlScalar dy, DlPath* dst) {
    path_.offset(dx, dy, &dst->path_);
  }

  void Transform(const DlTransform& matrix, DlPath* dst) {
    path_.transform(matrix.ToSkMatrix(), &dst->path_);
  }

  bool Contains(DlScalar x, DlScalar y) const { return path_.contains(x, y); }

  DlPath& AddRectLTRB(DlScalar left,
                      DlScalar top,
                      DlScalar right,
                      DlScalar bottom) {
    path_.addRect(left, top, right, bottom);
    return *this;
  }
  DlPath& AddRectXYWH(DlScalar x, DlScalar y, DlScalar width, DlScalar height) {
    path_.addRect(SkRect::MakeXYWH(x, y, width, height));
    return *this;
  }
  DlPath& AddRect(const DlFRect& rect) {
    path_.addRect(*reinterpret_cast<const SkRect*>(&rect));
    return *this;
  }

  DlPath& AddOvalLTRB(DlScalar left,
                      DlScalar top,
                      DlScalar right,
                      DlScalar bottom) {
    path_.addOval(SkRect::MakeLTRB(left, top, right, bottom));
    return *this;
  }
  DlPath& AddOvalXYWH(DlScalar x, DlScalar y, DlScalar width, DlScalar height) {
    path_.addOval(SkRect::MakeXYWH(x, y, width, height));
    return *this;
  }
  DlPath& AddOval(const DlFRect& rect) {
    path_.addOval(*reinterpret_cast<const SkRect*>(&rect));
    return *this;
  }

  DlPath& AddCircle(DlScalar center_x, DlScalar center_y, DlScalar radius) {
    path_.addCircle(center_x, center_y, radius);
    return *this;
  }

  DlPath& AddRoundRect(const DlFRect& rect,
                       DlScalar dx,
                       DlScalar dy,
                       Direction dir = Direction::kCW) {
    path_.addRoundRect(*reinterpret_cast<const SkRect*>(&rect), dx, dy,
                       static_cast<SkPathDirection>(dir));
    return *this;
  }

  DlPath& AddRoundRect(const DlFRRect& rect, Direction dir = Direction::kCW) {
    path_.addRRect(*reinterpret_cast<const SkRRect*>(&rect),
                   static_cast<SkPathDirection>(dir));
    return *this;
  }

  DlPath& AddArc(const DlFRect& rect, DlAngle startAngle, DlAngle sweepAngle) {
    path_.addArc(*reinterpret_cast<const SkRect*>(&rect),  //
                 startAngle.degrees(), sweepAngle.degrees());
    return *this;
  }

  DlPath& AddPolygon(const DlFPoint points[], int count, bool is_closed) {
    path_.addPoly(reinterpret_cast<const SkPoint*>(points), count, is_closed);
    return *this;
  }

  void Transform(const DlTransform& transform) {
    path_.transform(transform.ToSkMatrix());
  }

  DlFRect Bounds() const { return DlFRect::MakeBounds(path_.getBounds()); }

  const SkPath& GetSkiaPath() const { return path_; }

 private:
  SkPath path_;

  SkPath& GetMutableSkiaPath() { return path_; }
  friend class CanvasPath;
};

}  // namespace flutter

#endif  // FLUTTER_DISPLAY_LIST_GEOMETRY_DL_PATH_H_
