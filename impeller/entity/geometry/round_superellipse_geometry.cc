// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cmath>
#include <vector>

#include "flutter/impeller/entity/geometry/round_superellipse_geometry.h"

#include "impeller/geometry/constants.h"

namespace impeller {

static constexpr Scalar kRatio_N_DOverA_Theta[][4] = {
    {2.000, 2.00000, 0.00000, 0.26000},
    {2.020, 2.03300, 0.01441, 0.23845},
    {2.040, 2.06500, 0.02568, 0.20310},
    {2.060, 2.09800, 0.03655, 0.18593},
    {2.080, 2.13200, 0.04701, 0.17341},
    {2.100, 2.17800, 0.05596, 0.14049},
    {2.120, 2.19300, 0.06805, 0.17417},
    {2.140, 2.23000, 0.07733, 0.16145},
    {2.160, 2.26400, 0.08677, 0.15649},
    {2.180, 2.30500, 0.09529, 0.14374},
    {2.200, 2.32900, 0.10530, 0.15212},
    {2.220, 2.38300, 0.11230, 0.12974},
    {2.240, 2.39800, 0.12257, 0.14433},
    {2.260, 2.41800, 0.13236, 0.15439},
    {2.280, 2.47200, 0.13867, 0.13431},
    {2.300, 2.50900, 0.14649, 0.13021}
};

static constexpr size_t NUM_RECORDS = sizeof(kRatio_N_DOverA_Theta) / sizeof(kRatio_N_DOverA_Theta[0]);
static constexpr Scalar MAX_RATIO = kRatio_N_DOverA_Theta[NUM_RECORDS-1][0];
static constexpr Scalar RATIO_STEP = kRatio_N_DOverA_Theta[1][0] - kRatio_N_DOverA_Theta[0][0];

static constexpr Scalar gap(Scalar corner_radius) {
  return 0.2924303407 * corner_radius;
}

struct ExpandedVariables {
  Scalar n;
  Scalar d;
  Scalar R;
  Scalar x0;
  Scalar y0;
};

// Result will be assigned with [n, d_over_a, theta]
static ExpandedVariables ExpandVariables(Scalar ratio, Scalar a, Scalar g) {
  constexpr Scalar MIN_RATIO = kRatio_N_DOverA_Theta[0][0];
  Scalar steps = std::clamp<size_t>((ratio - MIN_RATIO) / RATIO_STEP, 0, NUM_RECORDS);
  size_t lo = std::min((size_t)std::floor(steps), NUM_RECORDS - 1);
  size_t hi = lo + 1;
  Scalar pos = steps - lo;

  Scalar n = pos * kRatio_N_DOverA_Theta[lo][1] + (1-pos) * kRatio_N_DOverA_Theta[hi][1];
  Scalar d = (pos * kRatio_N_DOverA_Theta[lo][2] + (1-pos) * kRatio_N_DOverA_Theta[hi][2]) * a;
  Scalar R = a - d - g;
  Scalar theta = pos * kRatio_N_DOverA_Theta[lo][3] + (1-pos) * kRatio_N_DOverA_Theta[hi][3];
  Scalar x0 = d + R * sin(theta);
  Scalar y0 = pow(pow(a, n) - pow(x0, n), 1 / n);
  return ExpandedVariables{
    .n = n,
    .d = d,
    .R = R,
    .x0 = x0,
    .y0 = y0,
  };
}

static void DrawCircularArc(Point start, Point end, Scalar r) {
  // TODO
}

RoundSuperellipseGeometry::RoundSuperellipseGeometry(const Rect& bounds,
                                           Scalar corner_radius)
    : bounds_(bounds),
      corner_radius_(corner_radius) {}

RoundSuperellipseGeometry::~RoundSuperellipseGeometry() {}

GeometryResult RoundSuperellipseGeometry::GetPositionBuffer(
    const ContentContext& renderer,
    const Entity& entity,
    RenderPass& pass) const {
  Size size = bounds_.GetSize();
  Point center = bounds_.GetCenter();
  // printf("Center %.2f, %.2f\n", center.x, center.y);
  Scalar r = std::min(corner_radius_, std::min(size.width / 2, size.height / 2));

  // Derive critical variables
  Size ratio_wh = {std::min(size.width / r, MAX_RATIO), std::min(size.height / r, MAX_RATIO)};
  Size ab = ratio_wh * r;
  Size s_wh = size / 2 - ab;
  Scalar g = gap(corner_radius_);

  ExpandedVariables var_w = ExpandVariables(ratio_wh.width, ab.width, g);
  ExpandedVariables var_h = ExpandVariables(ratio_wh.height, ab.height, g);
  Scalar c = (ab.width - size.height) / 2;

  /* Generate the points for the top right quadrant, and then mirror to the other
   * quadrants. The following figure shows the top 1/8 arc (from 0 to pi/4), which
   * is a square-like squircle offset by (0, -c).
   *
   *  straight  superelipse
   *      ↓        ↓
   *   A       B      J    ↙ circular arc
   *   -------------__、
   *   |       |     /  ヽ M
   *   |       |    /   ⟋ \
   *   |       |   / ⟋     \
   *   |       |  ᨀ         |
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
   *
   * The next 1/8 arc (from pi/4 to pi/2) has mirrored points:
   *
   *   J' = (y0_h + c, x0_h)
   *   B' = (w/2, s_h)
   *   A' = (w/2, 0)
   */

  // TODO(dkwingsmt): determine parameter values based on scaling factor.
  Scalar step = kPi / 80;

  std::vector<Point> points;
  points.reserve(41);
  Point pointM {size.width / 2 - g, size.height / 2 - g};

  // A
  points.emplace_back(0, size.height / 2);
  // B
  points.emplace_back(s_wh.width, size.height / 2);
  // Arc BJ (both ends exclusive)
  Scalar angle_jsb = atan((var_w.x0 - s_wh.width) / var_w.y0);
  for (Scalar angle = 0 + step; angle < angle_jsb; angle += step) {
    // printf("Angle %.2f\n", angle);
    Scalar x = ab.width * pow(abs(sin(angle)), 2 / var_w.n);
    Scalar y = ab.width * pow(abs(cos(angle)), 2 / var_w.n) - c;
    points.emplace_back(x, y);
  }
  // J
  points.emplace_back(var_w.x0, var_w.y0 - c);
  // Arc JM (both ends exclusive)
  DrawCircularArc({var_w.x0, var_w.y0 - c}, pointM, var_w.R);
  // M
  points.push_back(pointM);
  // Arc MJ' (both ends exclusive)
  DrawCircularArc(pointM, {var_h.y0 + c, var_h.x0}, var_h.R);
  // J'
  points.emplace_back(var_h.y0 + c, var_w.x0);
  // Arc BJ (both ends exclusive)
  Scalar angle_bsj = atan((var_h.x0 - s_wh.height) / var_h.y0);
  for (Scalar angle = angle_bsj - step; angle > 0; angle -= step) {
    // printf("Angle %.2f\n", angle);
    Scalar x = ab.height * pow(abs(cos(angle)), 2 / var_h.n) + c;
    Scalar y = ab.height * pow(abs(sin(angle)), 2 / var_h.n);
    points.emplace_back(x, y);
  }
  // A'
  points.emplace_back(size.width / 2, 0);

  static constexpr Point reflection[4] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}};

  // Reflect into the 4 quadrants and generate the tessellated mesh. The
  // iteration order is reversed so that the trianges are continuous from
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
