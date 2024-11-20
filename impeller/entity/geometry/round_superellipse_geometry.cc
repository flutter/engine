// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>
#include <ranges>
#include <vector>

#include "flutter/impeller/entity/geometry/round_superellipse_geometry.h"

#include "impeller/geometry/constants.h"

namespace impeller {

static constexpr double kRatio_N_DOverA_Theta[][4] = {
    {2.000, 2.00000, 0.00000, 0.26000},  //
    {2.020, 2.03300, 0.01441, 0.23845},  //
    {2.040, 2.06500, 0.02568, 0.20310},  //
    {2.060, 2.09800, 0.03655, 0.18593},  //
    {2.080, 2.13200, 0.04701, 0.17341},  //
    {2.100, 2.17800, 0.05596, 0.14049},  //
    {2.120, 2.19300, 0.06805, 0.17417},  //
    {2.140, 2.23000, 0.07733, 0.16145},  //
    {2.160, 2.26400, 0.08677, 0.15649},  //
    {2.180, 2.30500, 0.09529, 0.14374},  //
    {2.200, 2.32900, 0.10530, 0.15212},  //
    {2.220, 2.38300, 0.11230, 0.12974},  //
    {2.240, 2.39800, 0.12257, 0.14433},  //
    {2.260, 2.41800, 0.13236, 0.15439},  //
    {2.280, 2.47200, 0.13867, 0.13431},  //
    {2.300, 2.50900, 0.14649, 0.13021}   //
};

static constexpr size_t NUM_RECORDS =
    sizeof(kRatio_N_DOverA_Theta) / sizeof(kRatio_N_DOverA_Theta[0]);
static constexpr Scalar MIN_RATIO = kRatio_N_DOverA_Theta[0][0];
static constexpr double MAX_RATIO = kRatio_N_DOverA_Theta[NUM_RECORDS - 1][0];
static constexpr double RATIO_STEP =
    kRatio_N_DOverA_Theta[1][0] - kRatio_N_DOverA_Theta[0][0];

static constexpr double gap(double corner_radius) {
  return 0.2924303407 * corner_radius;
}

typedef TSize<double> DSize;
typedef TPoint<double> DPoint;
struct ExpandedVariables {
  double n;
  double d;
  double R;
  double x0;
  double y0;
};

// // Result will be assigned with [n, d_over_a, theta]
// static ExpandedVariables ExpandVariables(double ratio, double a, double g) {
//   constexpr Scalar MIN_RATIO = kRatio_N_DOverA_Theta[0][0];
//   double steps =
//       std::clamp<Scalar>((ratio - MIN_RATIO) / RATIO_STEP, 0, NUM_RECORDS -
//       1);
//   size_t lo = std::clamp<size_t>((size_t)std::floor(steps), 0, NUM_RECORDS -
//   2); size_t hi = lo + 1; double pos = steps - lo;

//   double n = (1 - pos) * kRatio_N_DOverA_Theta[lo][1] +
//              pos * kRatio_N_DOverA_Theta[hi][1];
//   double d = ((1 - pos) * kRatio_N_DOverA_Theta[lo][2] +
//               pos * kRatio_N_DOverA_Theta[hi][2]) *
//              a;
//   double R = (a - d - g) * sqrt(2);
//   double theta = (1 - pos) * kRatio_N_DOverA_Theta[lo][3] +
//                  pos * kRatio_N_DOverA_Theta[hi][3];
//   double x0 = d + R * sin(theta);
//   double y0 = pow(pow(a, n) - pow(x0, n), 1 / n);
//   return ExpandedVariables{
//       .n = n,
//       .d = d,
//       .R = R,
//       .x0 = x0,
//       .y0 = y0,
//   };
// }

static Point operator+(Point a, DPoint b) {
  return Point{static_cast<Scalar>(a.x + b.x), static_cast<Scalar>(a.y + b.y)};
}

// Draw a circular arc from `start` to `end` with a radius of `r`.
//
// It is assumed that `start` is north-west to `end`, and the center
// of the circle is south-west to both points.
static void DrawCircularArc(std::vector<DPoint>& output,
                            DPoint start,
                            DPoint end,
                            double r) {
  /* Denote the middle point of S and E as M. The key is to find the center of
   * the circle.
   *         S --__
   *          /  ⟍ `、
   *         /   M  ⟍\
   *        /       ⟋  E
   *       /     ⟋
   *      /   ⟋
   *     / ⟋
   *  C ⟋
   */

  const DPoint s_to_e = end - start;
  const DPoint m = (start + end) / 2;
  const DPoint c_to_m = DPoint(-s_to_e.y, s_to_e.x);
  const double distance_sm = s_to_e.GetLength() / 2;
  const double distance_cm = sqrt(r * r - distance_sm * distance_sm);
  const DPoint c = m - distance_cm * c_to_m.Normalize();
  ;
  const Scalar angle_sce = asinf(distance_sm / r) * 2;
  // TODO(dkwingsmt): determine parameter values based on scaling factor.
  Scalar step = kPi / 80;
  const DPoint c_to_s = start - c;
  for (Scalar angle = step; angle < angle_sce; angle += step) {
    output.push_back(c_to_s.Rotate(Radians(-angle)) + c);
  }
}

static void DrawOctantSquareLikeSquircle(std::vector<DPoint>& output,
                                         double size,
                                         double corner_radius,
                                         DPoint center,
                                         bool flip) {
  /* Generate the points for the top 1/8 arc (from 0 to pi/4), which
   * is a square-like squircle offset by (0, -c).
   *
   *  straight  superelipse
   *      ↓        ↓
   *   A       B      J    ↙ circular arc
   *   ------------..._
   *   |       |     /  `、 M
   *   |       |    /   ⟋ \
   *   |       |   / ⟋     \
   *   |       |  .⟋        |
   * O +       | / D        |
   *   |       |/           |
   *  E--------|------------|
   *           S
   *
   * Ignore the central offset until the last step, and assume point O, the
   * origin, is (0, 0),
   *
   *   A = (0, h/2)
   *   B = (s_w, h/2)
   *   J = (x0_w, y0_w - c)
   *   M = (w/2 - g, h/2 - g)
   */

  // Derive critical variables
  const double ratio = {std::min(size / corner_radius, MAX_RATIO)};
  const double a = ratio * corner_radius / 2;
  const double s = size / 2 - a;
  const double g = gap(corner_radius);

  // Use look up table to derive critical variables
  double steps =
      std::clamp<Scalar>((ratio - MIN_RATIO) / RATIO_STEP, 0, NUM_RECORDS - 1);
  size_t lo = std::clamp<size_t>((size_t)std::floor(steps), 0, NUM_RECORDS - 2);
  size_t hi = lo + 1;
  double pos = steps - lo;

  double n = (1 - pos) * kRatio_N_DOverA_Theta[lo][1] +
             pos * kRatio_N_DOverA_Theta[hi][1];
  double d = ((1 - pos) * kRatio_N_DOverA_Theta[lo][2] +
              pos * kRatio_N_DOverA_Theta[hi][2]) *
             a;
  double R = (a - d - g) * sqrt(2);
  double theta = (1 - pos) * kRatio_N_DOverA_Theta[lo][3] +
                 pos * kRatio_N_DOverA_Theta[hi][3];
  double x0 = d + R * sin(theta);
  double y0 = pow(pow(a, n) - pow(x0, n), 1 / n);

  const DPoint pointM{size / 2 - g, size / 2 - g};

  // Points without applying `flip` and `center`
  std::vector<DPoint> points;

  // A
  points.emplace_back(0, size / 2);
  // Superellipsoid arc BJ (B inclusive, J exclusive)
  {
    // TODO(dkwingsmt): determine parameter values based on scaling factor.
    constexpr Scalar step = kPi / 80;
    const Scalar target_slope = y0 / x0;
    for (Scalar angle = 0;; angle += step) {
      const double x = a * pow(abs(sinf(angle)), 2 / n);
      const double y = a * pow(abs(cosf(angle)), 2 / n);
      if (y <= target_slope * x) {
        break;
      }
      points.emplace_back(x + s, y + s);
    }
  }
  // J
  points.emplace_back(x0 + s, y0 + s);
  // Circular arc JM (both ends exclusive)
  DrawCircularArc(points, {x0 + s, y0 + s}, pointM, R);

  if (!flip) {
    for (const DPoint& point : points) {
      output.push_back(point + center);
    }
  } else {
    for (size_t i = 0; i < points.size(); i++) {
      const DPoint& point = points[points.size() - i - 1];
      output.emplace_back(point.y + center.x, point.x + center.y);
    }
  }
}

static Scalar LimitRadius(Scalar corner_radius, const Rect& bounds) {
  return std::min(corner_radius,
                  std::min(bounds.GetWidth() / 2, bounds.GetHeight() / 2));
}

RoundSuperellipseGeometry::RoundSuperellipseGeometry(const Rect& bounds,
                                                     Scalar corner_radius)
    : bounds_(bounds), corner_radius_(LimitRadius(corner_radius, bounds)) {}

RoundSuperellipseGeometry::~RoundSuperellipseGeometry() {}

GeometryResult RoundSuperellipseGeometry::GetPositionBuffer(
    const ContentContext& renderer,
    const Entity& entity,
    RenderPass& pass) const {
  DSize size{bounds_.GetWidth(), bounds_.GetHeight()};
  Point center = bounds_.GetCenter();

  DSize ab = size / 2;
  const double c = ab.width - size.height / 2;

  // Points for the first quadrant.
  std::vector<DPoint> points;
  points.reserve(41);

  DrawOctantSquareLikeSquircle(points, size.width, corner_radius_,
                               DPoint{0, -c}, false);
  points.push_back(DPoint(size / 2) - gap(corner_radius_));  // Point M
  DrawOctantSquareLikeSquircle(points, size.height, corner_radius_,
                               DPoint{c, 0}, true);

  static constexpr DPoint reflection[4] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};

  // Reflect the 1/4 arc into the 4 quadrants and generate the tessellated mesh.
  // The iteration order is reversed so that the trianges are continuous from
  // quadrant to quadrant.
  std::vector<Point> geometry;
  geometry.reserve(1 + 4 * points.size());
  geometry.push_back(center);
  // All arcs include the starting point and exclude the ending point.
  for (auto i = 0u; i < points.size() - 1; i++) {
    geometry.push_back(center + (reflection[0] * points[i]));
  }
  for (auto i = points.size() - 1; i >= 1; i--) {
    geometry.push_back(center + (reflection[1] * points[i]));
  }
  for (auto i = 0u; i < points.size() - 1; i++) {
    geometry.push_back(center + (reflection[2] * points[i]));
  }
  for (auto i = points.size() - 1; i >= 1; i--) {
    geometry.push_back(center + (reflection[3] * points[i]));
  }
  geometry.push_back(center + points[0]);

  std::vector<uint16_t> indices;
  indices.reserve(geometry.size() * 3);
  for (auto i = 2u; i < geometry.size(); i++) {
    indices.push_back(0);
    indices.push_back(i - 1);
    indices.push_back(i);
  }

  auto& host_buffer = renderer.GetTransientsBuffer();
  return GeometryResult{
      .type = PrimitiveType::kTriangle,
      .vertex_buffer =
          {
              .vertex_buffer = host_buffer.Emplace(
                  geometry.data(), geometry.size() * sizeof(Point),
                  alignof(Point)),
              .index_buffer = host_buffer.Emplace(
                  indices.data(), indices.size() * sizeof(uint16_t),
                  alignof(uint16_t)),
              .vertex_count = indices.size(),
              .index_type = IndexType::k16bit,
          },
      .transform = entity.GetShaderTransform(pass),
  };
}

std::optional<Rect> RoundSuperellipseGeometry::GetCoverage(
    const Matrix& transform) const {
  return bounds_.TransformBounds(transform);
}

bool RoundSuperellipseGeometry::CoversArea(const Matrix& transform,
                                           const Rect& rect) const {
  return false;
}

bool RoundSuperellipseGeometry::IsAxisAlignedRect() const {
  return false;
}

}  // namespace impeller
