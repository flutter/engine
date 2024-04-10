// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_GEOMETRY_PATH_COMPONENT_H_
#define FLUTTER_IMPELLER_GEOMETRY_PATH_COMPONENT_H_

#include <functional>
#include <type_traits>
#include <variant>
#include <vector>

#include "impeller/geometry/point.h"
#include "impeller/geometry/rect.h"
#include "impeller/geometry/scalar.h"

namespace impeller {

Scalar ApproximateParabolaIntegral(Scalar x);

constexpr Scalar FastHypot(Scalar a, Scalar b) {
  return b + 0.337f * a;
}

// The default tolerance value for QuadraticCurveComponent::AppendPolylinePoints
// and CubicCurveComponent::AppendPolylinePoints. It also impacts the number of
// quadratics created when flattening a cubic curve to a polyline.
//
// Smaller numbers mean more points. This number seems suitable for particularly
// curvy curves at scales close to 1.0. As the scale increases, this number
// should be divided by Matrix::GetMaxBasisLength to avoid generating too few
// points for the given scale.
static constexpr Scalar kDefaultCurveTolerance = .1f;

struct LinearPathComponent {
  Point p1;
  Point p2;

  LinearPathComponent() {}

  LinearPathComponent(Point ap1, Point ap2) : p1(ap1), p2(ap2) {}

  Point Solve(Scalar time) const;

  void AppendPolylinePoints(std::vector<Point>& points) const;

  std::vector<Point> Extrema() const;

  bool operator==(const LinearPathComponent& other) const {
    return p1 == other.p1 && p2 == other.p2;
  }

  std::optional<Vector2> GetStartDirection() const;

  std::optional<Vector2> GetEndDirection() const;
};

// A component that represets a Quadratic Bézier curve.
struct QuadraticPathComponent {
  // Start point.
  Point p1;
  // Control point.
  Point cp;
  // End point.
  Point p2;

  QuadraticPathComponent() {}

  QuadraticPathComponent(Point ap1, Point acp, Point ap2)
      : p1(ap1), cp(acp), p2(ap2) {}

  Point Solve(Scalar time) const;

  Point SolveDerivative(Scalar time) const;

  // Uses the algorithm described by Raph Levien in
  // https://raphlinus.github.io/graphics/curves/2019/12/23/flatten-quadbez.html.
  //
  // The algorithm has several benefits:
  // - It does not require elevation to cubics for processing.
  // - It generates fewer and more accurate points than recursive subdivision.
  // - Each turn of the core iteration loop has no dependencies on other turns,
  //   making it trivially parallelizable.
  //
  // See also the implementation in kurbo: https://github.com/linebender/kurbo.
  void AppendPolylinePoints(Scalar scale_factor,
                            std::vector<Point>& points) const;

  using PointProc = std::function<void(const Point& point)>;

  void ToLinearPathComponents(Scalar scale_factor, const PointProc& proc) const;

  template <typename VertexWriter>
  void WriteLinearPathComponents(Scalar scale_factor,
                                 VertexWriter& writer) const {
    Scalar tolerance = kDefaultCurveTolerance / scale_factor;
    Scalar sqrt_tolerance = sqrt(tolerance);

    Point d01 = cp - p1;
    Point d12 = p2 - cp;
    Point dd = d01 - d12;
    Scalar cross = (p2 - p1).Cross(dd);
    Scalar x0 = d01.Dot(dd) * 1.0f / cross;
    Scalar x2 = d12.Dot(dd) * 1.0f / cross;
    Scalar scale = std::abs(cross / (FastHypot(dd.x, dd.y) * (x2 - x0)));

    Scalar a0 = ApproximateParabolaIntegral(x0);
    Scalar a2 = ApproximateParabolaIntegral(x2);
    Scalar val = 0.f;
    // if (std::isfinite(scale)) {
      Scalar da = std::abs(a2 - a0);
      Scalar sqrt_scale = sqrt(scale);
      if ((x0 < 0 && x2 < 0) || (x0 >= 0 && x2 >= 0)) {
        val = da * sqrt_scale;
      } else {
        // cusp case
        Scalar xmin = sqrt_tolerance / sqrt_scale;
        val = sqrt_tolerance * da / ApproximateParabolaIntegral(xmin);
      }
    // }
    Scalar u0 = ApproximateParabolaIntegral(a0);
    Scalar u2 = ApproximateParabolaIntegral(a2);
    Scalar uscale = 1.0f / (u2 - u0);

    Scalar line_count = std::max(1.0f, ceil(0.5f * val / sqrt_tolerance));
    Scalar step = 1.0f / line_count;
    for (size_t i = 1; i < line_count; i += 1) {
      Scalar u = i * step;
      Scalar a = a0 + (a2 - a0) * u;
      Scalar t = (ApproximateParabolaIntegral(a) - u0) * uscale;
      writer.Write(Solve(t));
    }
    writer.Write(p2);
  }

  std::vector<Point> Extrema() const;

  bool operator==(const QuadraticPathComponent& other) const {
    return p1 == other.p1 && cp == other.cp && p2 == other.p2;
  }

  std::optional<Vector2> GetStartDirection() const;

  std::optional<Vector2> GetEndDirection() const;
};

// A component that represets a Cubic Bézier curve.
struct CubicPathComponent {
  // Start point.
  Point p1;
  // The first control point.
  Point cp1;
  // The second control point.
  Point cp2;
  // End point.
  Point p2;

  CubicPathComponent() {}

  explicit CubicPathComponent(const QuadraticPathComponent& q)
      : p1(q.p1),
        cp1(q.p1 + (q.cp - q.p1) * (2.0 / 3.0)),
        cp2(q.p2 + (q.cp - q.p2) * (2.0 / 3.0)),
        p2(q.p2) {}

  CubicPathComponent(Point ap1, Point acp1, Point acp2, Point ap2)
      : p1(ap1), cp1(acp1), cp2(acp2), p2(ap2) {}

  Point Solve(Scalar time) const;

  Point SolveDerivative(Scalar time) const;

  // This method approximates the cubic component with quadratics, and then
  // generates a polyline from those quadratics.
  //
  // See the note on QuadraticPathComponent::AppendPolylinePoints for
  // references.
  void AppendPolylinePoints(Scalar scale, std::vector<Point>& points) const;

  std::vector<Point> Extrema() const;

  using PointProc = std::function<void(const Point& point)>;

  void ToLinearPathComponents(Scalar scale, const PointProc& proc) const;

  template <typename VertexWriter>
  void WriteLinearPathComponents(Scalar scale_factor,
                                 VertexWriter& writer) const {
    constexpr Scalar accuracy = 0.1f;
    // The maximum error, as a vector from the cubic to the best approximating
    // quadratic, is proportional to the third derivative, which is constant
    // across the segment. Thus, the error scales down as the third power of
    // the number of subdivisions. Our strategy then is to subdivide `t` evenly.
    //
    // This is an overestimate of the error because only the component
    // perpendicular to the first derivative is important. But the simplicity is
    // appealing.

    // This magic number is the square of 36 / sqrt(3).
    // See: http://caffeineowl.com/graphics/2d/vectorial/cubic2quad01.html
    Scalar max_hypot2 = 432.0f * accuracy * accuracy;
    Point p1x2 = 3.0f * cp1 - p1;
    Point p2x2 = 3.0f * cp2 - p2;
    Point p = p2x2 - p1x2;
    Scalar err = p.Dot(p);
    Scalar quad_count = std::max(1.0f, ceil(pow(err / max_hypot2, 1.0f / 6.0f)));
    for (size_t i = 0; i < quad_count; i++) {
      Scalar t0 = i / quad_count;
      Scalar t1 = (i + 1) / quad_count;
      CubicPathComponent seg = Subsegment(t0, t1);
      Point p1x2 = 3.0 * seg.cp1 - seg.p1;
      Point p2x2 = 3.0 * seg.cp2 - seg.p2;
      QuadraticPathComponent(seg.p1, ((p1x2 + p2x2) / 4.0), seg.p2)
          .WriteLinearPathComponents<VertexWriter>(scale_factor, writer);
    }
  }

  CubicPathComponent Subsegment(Scalar t0, Scalar t1) const;

  bool operator==(const CubicPathComponent& other) const {
    return p1 == other.p1 && cp1 == other.cp1 && cp2 == other.cp2 &&
           p2 == other.p2;
  }

  std::optional<Vector2> GetStartDirection() const;

  std::optional<Vector2> GetEndDirection() const;

 private:
  QuadraticPathComponent Lower() const;
};

struct ContourComponent {
  Point destination;
  bool is_closed = false;

  ContourComponent() {}

  explicit ContourComponent(Point p, bool is_closed = false)
      : destination(p), is_closed(is_closed) {}

  bool operator==(const ContourComponent& other) const {
    return destination == other.destination && is_closed == other.is_closed;
  }
};

using PathComponentVariant = std::variant<std::monostate,
                                          const LinearPathComponent*,
                                          const QuadraticPathComponent*,
                                          const CubicPathComponent*>;

struct PathComponentStartDirectionVisitor {
  std::optional<Vector2> operator()(const LinearPathComponent* component);
  std::optional<Vector2> operator()(const QuadraticPathComponent* component);
  std::optional<Vector2> operator()(const CubicPathComponent* component);
  std::optional<Vector2> operator()(std::monostate monostate) {
    return std::nullopt;
  }
};

struct PathComponentEndDirectionVisitor {
  std::optional<Vector2> operator()(const LinearPathComponent* component);
  std::optional<Vector2> operator()(const QuadraticPathComponent* component);
  std::optional<Vector2> operator()(const CubicPathComponent* component);
  std::optional<Vector2> operator()(std::monostate monostate) {
    return std::nullopt;
  }
};

static_assert(!std::is_polymorphic<LinearPathComponent>::value);
static_assert(!std::is_polymorphic<QuadraticPathComponent>::value);
static_assert(!std::is_polymorphic<CubicPathComponent>::value);

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_GEOMETRY_PATH_COMPONENT_H_
