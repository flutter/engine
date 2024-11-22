// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>
#include <ranges>
#include <vector>

#include "flutter/impeller/entity/geometry/round_superellipse_geometry.h"

#include "impeller/geometry/constants.h"

namespace impeller {

// A look up table with precomputed variables.
//
// The columns represent the following variabls respectively:
//
//  * ratio = size / a
//  * n
//  * d / a
//  * theta
//
// For definition of the variables, see DrawOctantSquareLikeSquircle.
static constexpr Scalar kPrecomputedVariables[][4] = {
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

static constexpr size_t kNumRecords =
    sizeof(kPrecomputedVariables) / sizeof(kPrecomputedVariables[0]);
static constexpr Scalar kMinRatio = kPrecomputedVariables[0][0];
static constexpr Scalar kMaxRatio = kPrecomputedVariables[kNumRecords - 1][0];
static constexpr Scalar kRatioStep =
    kPrecomputedVariables[1][0] - kPrecomputedVariables[0][0];

static constexpr Scalar kAngleStep = kPi / 80;

static constexpr Scalar gap(Scalar corner_radius) {
  return 0.2924303407 * corner_radius;
}

// Draw a circular arc from `start` to `end` with a radius of `r`.
//
// It is assumed that `start` is north-west to `end`, and the center
// of the circle is south-west to both points.
//
// The resulting points are appended to `output` and include the starting point
// but exclude the ending point.
static void DrawCircularArc(std::vector<Point>& output,
                            Point start,
                            Point end,
                            Scalar r) {
  /* Denote the middle point of S and E as M. The key is to find the center of
   * the circle.
   *         S --__
   *          /  ⟍ `、
   *         /   M  ⟍\
   *        /       ⟋  E
   *       /     ⟋   ↗
   *      /   ⟋
   *     / ⟋    r
   *  C ⟋  ↙
   */

  Point s_to_e = end - start;
  Point m = (start + end) / 2;
  Point c_to_m = Point(-s_to_e.y, s_to_e.x);
  Scalar distance_sm = s_to_e.GetLength() / 2;
  Scalar distance_cm = sqrt(r * r - distance_sm * distance_sm);
  Point c = m - distance_cm * c_to_m.Normalize();
  Scalar angle_sce = asinf(distance_sm / r) * 2;
  Point c_to_s = start - c;

  Scalar angle = 0;
  while (angle < angle_sce) {
    output.push_back(c_to_s.Rotate(Radians(-angle)) + c);
    angle += kAngleStep;
  }
}

static Scalar lerp(size_t column, size_t left, size_t frac) {
  return (1 - frac) * kPrecomputedVariables[left][column] +
         frac * kPrecomputedVariables[left + 1][column];
}

// Draws an arc representing the top 1/8 segment of a square-like rounded
// superellipse. The arc spans from 0 to pi/4, moving clockwise starting from
// the positive Y-axis.
//
// The full square-like rounded superellipse has a width and height specified by
// `size`. It is centered at `center` and features rounded corners determined by
// `corner_radius`. The `corner_radius` corresponds to the `cornerRadius`
// parameter in SwiftUI, rather than the literal radius of corner circles.
//
// If `flip` is true, the function instead produces the next 1/8 arc, spanning
// from pi/4 to pi/8. Technically, the X and Y coordinates of the arc points
// are swapped before applying `center`, and their order is reversed as well.
//
// The list of the resulting points is appended to `output`, and includes the
// starting point but exclude the ending point.
static void DrawOctantSquareLikeSquircle(std::vector<Point>& output,
                                         Scalar size,
                                         Scalar corner_radius,
                                         Point center,
                                         bool flip) {
  /* Ignoring `center` and `flip`, the following figure shows the first quadrant
   * of a square-like rounded superellipse. The target arc consists of the
   * "stretch" (AB), a superellipsoid arc (BJ), and a circular arc (JM).
   *
   * Define gap (g) as the distance between point M and the bounding box,
   * therefore point M is at (size/2 - g, size/2 - g). Assume the coordinate of
   * J is (xJ, yJ).
   *
   *     straight   superelipse
   *          ↓     ↓
   *        A    B      J     circular arc
   *        ---------...._↙
   *        |    |      /  `⟍ M
   *        |    |     /    ⟋ ⟍
   *        |    |    /θ ⟋     \
   *        |    |   /◝⟋        |
   *        |    |  ᜱ           |
   *        |    | /  D          |
   *    ↑   +----+               |
   *    s   |    |               |
   *    ↓   +----+---------------|
   *       O     S
   *        ← s →
   *        ←------ size/2 ------→
   */

  Scalar ratio = {std::min(size / corner_radius, kMaxRatio)};
  Scalar a = ratio * corner_radius / 2;
  Scalar s = size / 2 - a;
  Scalar g = gap(corner_radius);

  // Use look up table to derive critical variables
  Scalar steps =
      std::clamp<Scalar>((ratio - kMinRatio) / kRatioStep, 0, kNumRecords - 1);
  size_t left =
      std::clamp<size_t>((size_t)std::floor(steps), 0, kNumRecords - 2);
  Scalar frac = steps - left;
  Scalar n = lerp(1, left, frac);
  Scalar d = lerp(2, left, frac) * a;
  Scalar theta = lerp(3, left, frac);

  Scalar R = (a - d - g) * sqrt(2);
  Scalar xJ = d + R * sin(theta);
  Scalar yJ = pow(pow(a, n) - pow(xJ, n), 1 / n);

  Point pointM{size / 2 - g, size / 2 - g};

  // Points without applying `flip` and `center`.
  std::vector<Point> points;
  points.reserve(21);

  // A
  points.emplace_back(0, size / 2);
  // Superellipsoid arc BJ (B inclusive, J exclusive)
  // https://math.stackexchange.com/questions/2573746/superellipse-parametric-equation
  {
    const Scalar target_slope = yJ / xJ;
    Scalar angle = 0;
    while (true) {
      Scalar x = a * pow(abs(sinf(angle)), 2 / n);
      Scalar y = a * pow(abs(cosf(angle)), 2 / n);
      if (y <= target_slope * x) {
        break;
      }
      points.emplace_back(x + s, y + s);
      angle += kAngleStep;
    }
  }
  // Circular arc JM (B inclusive, M exclusive)
  DrawCircularArc(points, {xJ + s, yJ + s}, pointM, R);

  // Apply `flip` and `center`.
  if (!flip) {
    for (const Point& point : points) {
      output.push_back(point + center);
    }
  } else {
    for (size_t i = 0; i < points.size(); i++) {
      const Point& point = points[points.size() - i - 1];
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
  const Size size = bounds_.GetSize();
  const Point center = bounds_.GetCenter();

  // The full shape is divided into 4 segments: the top and bottom edges come
  // from two square-like rounded superellipses (called "width-aligned"), while
  // the left and right squircles come from another two ("height-aligned").
  //
  // Denote the distance from the center of the square-like squircles to the
  // origin as `c`. The width-aligned square-like squircle and the
  // height-aligned one have the same offset in different directions.
  const Scalar c = (size.width - size.height) / 2;

  // Draw the first quadrant of the shape and store in `points`. It will be
  // mirrored to other quadrants later.
  std::vector<Point> points;
  points.reserve(41);

  DrawOctantSquareLikeSquircle(points, size.width, corner_radius_, Point{0, -c},
                               false);
  points.push_back(Point(size / 2) - gap(corner_radius_));  // Point M
  DrawOctantSquareLikeSquircle(points, size.height, corner_radius_, Point{c, 0},
                               true);

  static constexpr Point reflection[4] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};

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
