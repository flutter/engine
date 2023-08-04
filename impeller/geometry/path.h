// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <optional>
#include <tuple>
#include <vector>

#include "impeller/geometry/path_component.h"

namespace impeller {

enum class Cap {
  kButt,
  kRound,
  kSquare,
};

enum class Join {
  kMiter,
  kRound,
  kBevel,
};

enum class FillType {
  kNonZero,  // The default winding order.
  kOdd,
  kPositive,
  kNegative,
  kAbsGeqTwo,
};

enum class Convexity {
  kUnknown,
  kConvex,
};

//------------------------------------------------------------------------------
/// @brief      Paths are lightweight objects that describe a collection of
///             linear, quadratic, or cubic segments. These segments may be
///             broken up by move commands, which are effectively linear
///             commands that pick up the pen rather than continuing to draw.
///
///             All shapes supported by Impeller are paths either directly or
///             via approximation (in the case of circles).
///
///             Creating paths that describe complex shapes is usually done by a
///             path builder.
///
///             Paths are internally reference counted. A copied path will
///             share underlying storage with the original path.
///             To make an independent copy use Path::Clone().
class Path {
 public:
  enum class ComponentType {
    kLinear,
    kQuadratic,
    kCubic,
    kContour,
  };

  struct PolylineContour {
    /// Index that denotes the first point of this contour.
    size_t start_index;
    /// Denotes whether the last point of this contour is connected to the first
    /// point of this contour or not.
    bool is_closed;

    /// The direction of the contour's start cap.
    Vector2 start_direction;
    /// The direction of the contour's end cap.
    Vector2 end_direction;
  };

  /// One or more contours represented as a series of points and indices in
  /// the point vector representing the start of a new contour.
  class Polyline {
   public:
    Polyline(std::vector<Point>&& points,
             std::vector<PolylineContour>&& countours,
             Scalar scale)
        : state_(std::make_shared<State>()) {
      state_->points = std::move(points);
      state_->contours = std::move(countours);
      state_->scale = scale;
    }
    /// Points in the polyline, which may represent multiple contours
    /// specified by indices in |breaks|.
    const std::vector<Point>& points() const { return state_->points; }
    const std::vector<PolylineContour>& contours() const {
      return state_->contours;
    }
    /// Convenience method to compute the start (inclusive) and end
    /// (exclusive) point of the given contour index.
    ///
    /// The contour_index parameter is clamped to contours.size().
    std::tuple<size_t, size_t> GetContourPointBounds(
        size_t contour_index) const {
      return state_->GetContourPointBounds(contour_index);
    }

   private:
    friend class Tessellator;
    friend class Path;

    struct State {
      std::vector<Point> points;
      std::vector<PolylineContour> contours;
      std::tuple<size_t, size_t> GetContourPointBounds(
          size_t contour_index) const;
      Scalar scale;

      struct TesselatorData {
        std::vector<float> vertices;
        std::vector<uint16_t> indices;
      };
      std::unordered_map<FillType, TesselatorData> tesselator_cache_;
    };
    std::shared_ptr<State> state_;
  };

  Path();

  ~Path();

  /// Creates a clone of this path. Modifications of cloned paths do not affect
  /// the original.
  Path Clone() const {
    Path copy;
    *copy.state_ = *state_;
    return copy;
  }

  size_t GetComponentCount(std::optional<ComponentType> type = {}) const {
    return state_->GetComponentCount(type);
  }

  void SetFillType(FillType fill) { state_->SetFillType(fill); }

  FillType GetFillType() const { return state_->GetFillType(); }

  bool IsConvex() const { return state_->IsConvex(); }

  Path& AddLinearComponent(Point p1, Point p2) {
    state_->AddLinearComponent(p1, p2);
    return *this;
  }

  Path& AddQuadraticComponent(Point p1, Point cp, Point p2) {
    state_->AddQuadraticComponent(p1, cp, p2);
    return *this;
  }

  Path& AddCubicComponent(Point p1, Point cp1, Point cp2, Point p2) {
    state_->AddCubicComponent(p1, cp1, cp2, p2);
    return *this;
  }

  Path& AddContourComponent(Point destination, bool is_closed = false) {
    state_->AddContourComponent(destination, is_closed);
    return *this;
  }

  void SetContourClosed(bool is_closed) { state_->SetContourClosed(is_closed); }

  template <class T>
  using Applier = std::function<void(size_t index, const T& component)>;
  void EnumerateComponents(
      const Applier<LinearPathComponent>& linear_applier,
      const Applier<QuadraticPathComponent>& quad_applier,
      const Applier<CubicPathComponent>& cubic_applier,
      const Applier<ContourComponent>& contour_applier) const {
    state_->EnumerateComponents(linear_applier, quad_applier, cubic_applier,
                                contour_applier);
  }

  bool GetLinearComponentAtIndex(size_t index,
                                 LinearPathComponent& linear) const {
    return state_->GetLinearComponentAtIndex(index, linear);
  }

  bool GetQuadraticComponentAtIndex(size_t index,
                                    QuadraticPathComponent& quadratic) const {
    return state_->GetQuadraticComponentAtIndex(index, quadratic);
  }

  bool GetCubicComponentAtIndex(size_t index, CubicPathComponent& cubic) const {
    return state_->GetCubicComponentAtIndex(index, cubic);
  }

  bool GetContourComponentAtIndex(size_t index,
                                  ContourComponent& contour) const {
    return state_->GetContourComponentAtIndex(index, contour);
  }

  bool UpdateLinearComponentAtIndex(size_t index,
                                    const LinearPathComponent& linear) {
    return state_->UpdateLinearComponentAtIndex(index, linear);
  }

  bool UpdateQuadraticComponentAtIndex(
      size_t index,
      const QuadraticPathComponent& quadratic) {
    return state_->UpdateQuadraticComponentAtIndex(index, quadratic);
  }

  bool UpdateCubicComponentAtIndex(size_t index, CubicPathComponent& cubic) {
    return state_->UpdateCubicComponentAtIndex(index, cubic);
  }

  bool UpdateContourComponentAtIndex(size_t index,
                                     const ContourComponent& contour) {
    return state_->UpdateContourComponentAtIndex(index, contour);
  }

  /// Callers must provide the scale factor for how this path will be
  /// transformed.
  ///
  /// It is suitable to use the max basis length of the matrix used to transform
  /// the path. If the provided scale is 0, curves will revert to lines.
  Polyline CreatePolyline(Scalar scale) const {
    return state_->CreatePolyline(scale);
  }

  std::optional<Rect> GetBoundingBox() const {
    return state_->GetBoundingBox();
  }

  std::optional<Rect> GetTransformedBoundingBox(const Matrix& transform) const {
    return state_->GetTransformedBoundingBox(transform);
  }

  std::optional<std::pair<Point, Point>> GetMinMaxCoveragePoints() const {
    return state_->GetMinMaxCoveragePoints();
  }

 private:
  friend class PathBuilder;

  void SetConvexity(Convexity value) { state_->SetConvexity(value); }

  struct State {
    struct ComponentIndexPair {
      ComponentType type = ComponentType::kLinear;
      size_t index = 0;

      ComponentIndexPair() {}

      ComponentIndexPair(ComponentType a_type, size_t a_index)
          : type(a_type), index(a_index) {}
    };

    size_t GetComponentCount(std::optional<ComponentType> type = {}) const;

    void SetFillType(FillType fill);

    FillType GetFillType() const;

    bool IsConvex() const;

    void AddLinearComponent(Point p1, Point p2);

    void AddQuadraticComponent(Point p1, Point cp, Point p2);

    void AddCubicComponent(Point p1, Point cp1, Point cp2, Point p2);

    void AddContourComponent(Point destination, bool is_closed = false);

    void SetContourClosed(bool is_closed);

    template <class T>
    using Applier = std::function<void(size_t index, const T& component)>;
    void EnumerateComponents(
        const Applier<LinearPathComponent>& linear_applier,
        const Applier<QuadraticPathComponent>& quad_applier,
        const Applier<CubicPathComponent>& cubic_applier,
        const Applier<ContourComponent>& contour_applier) const;

    bool GetLinearComponentAtIndex(size_t index,
                                   LinearPathComponent& linear) const;

    bool GetQuadraticComponentAtIndex(size_t index,
                                      QuadraticPathComponent& quadratic) const;

    bool GetCubicComponentAtIndex(size_t index,
                                  CubicPathComponent& cubic) const;

    bool GetContourComponentAtIndex(size_t index,
                                    ContourComponent& contour) const;

    bool UpdateLinearComponentAtIndex(size_t index,
                                      const LinearPathComponent& linear);

    bool UpdateQuadraticComponentAtIndex(
        size_t index,
        const QuadraticPathComponent& quadratic);

    bool UpdateCubicComponentAtIndex(size_t index, CubicPathComponent& cubic);

    bool UpdateContourComponentAtIndex(size_t index,
                                       const ContourComponent& contour);

    Polyline CreatePolyline(Scalar scale) const;

    std::optional<Rect> GetBoundingBox() const;

    std::optional<Rect> GetTransformedBoundingBox(
        const Matrix& transform) const;

    std::optional<std::pair<Point, Point>> GetMinMaxCoveragePoints() const;

    void SetConvexity(Convexity value);

    FillType fill_ = FillType::kNonZero;
    Convexity convexity_ = Convexity::kUnknown;
    std::vector<ComponentIndexPair> components_;
    std::vector<LinearPathComponent> linears_;
    std::vector<QuadraticPathComponent> quads_;
    std::vector<CubicPathComponent> cubics_;
    std::vector<ContourComponent> contours_;

    void InvalidateCache() {
      cached_bounding_box_.reset();
      cached_polyline_.reset();
    }

    mutable std::optional<std::optional<Rect>> cached_bounding_box_;
    mutable std::optional<Polyline> cached_polyline_;
  };

  std::shared_ptr<State> state_;
};

}  // namespace impeller
