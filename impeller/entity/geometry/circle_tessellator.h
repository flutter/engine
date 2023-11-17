// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <vector>

#include "flutter/impeller/geometry/matrix.h"
#include "flutter/impeller/geometry/point.h"
#include "flutter/impeller/geometry/scalar.h"

namespace impeller {

/// @brief  A structure to store the sine and cosine of an angle.
struct Trig {
  /// Construct a Trig object from a given angle in radians.
  Trig(Radians r) : cos(std::cos(r.radians)), sin(std::sin(r.radians)) {}

  /// Construct a Trig object from the given cosine and sine values.
  Trig(double cos, double sin) : cos(cos), sin(sin) {}

  double cos;
  double sin;

  Vector2 operator*(Scalar radius) const {
    return Vector2(cos * radius, sin * radius);
  }

  Vector2 interpolate(Vector2 start_vector, Vector2 end_vector) {
    return start_vector * cos + end_vector * sin;
  }
};

using TessellatedPointProc = std::function<void(const Point& p)>;

/// @brief  A utility class to compute the number of divisions for a circle
///         given a transform-adjusted pixel radius and methods for generating
///         a tessellated set of triangles for a quarter or full circle.
///
///         The constructor will compute the device pixel radius size for
///         the specified geometry-space |radius| when viewed under
///         a specified geometry-to-device |transform|.
///
///         The object should be constructed with the expected transform and
///         radius of the circle, but can then be used to generate a triangular
///         tessellation with the computed number of divisions for any
///         radius after that. Since the coordinate space in which the
///         circle being tessellated is not necessarily device pixel space,
///         the radius supplied during tessellation might not match the
///         pixel radius computed during construction, but the two values
///         should be related by the transform in place when the tessellated
///         triangles are rendered for maximum tessellation fidelity.
class CircleTessellator {
 public:
  /// @brief   Constructs a CircleDivider that produces enough segments to
  ///          reasonably approximate a circle with a specified |radius|
  ///          when viewed under the specified |transform|.
  constexpr CircleTessellator(const Matrix& transform, Scalar radius)
      : CircleTessellator(transform.GetMaxBasisLength() * radius) {}

  ~CircleTessellator() = default;

  /// @brief   Return the number of divisions computed by the algorithm for
  ///          a single quarter circle.
  size_t GetQuadrantDivisionCount() const { return quadrant_divisions_; }

  /// @brief   Return the number of vertices that will be generated to
  ///          tessellate a full circle with a triangle strip.
  ///
  ///          This value can be used to pre-allocate space in a vector
  ///          to hold the vertices that will be produced by the
  ///          |GenerateCircleTriangleStrip| and
  ///          |GenerateRoundCapLineTriangleStrip| methods.
  size_t GetCircleVertexCount() const { return (quadrant_divisions_ + 1) * 4; }

  /// @brief   Generate the vertices for a triangle strip that covers the
  ///          circle at a given |radius| from a given |center|, delivering
  ///          the computed coordinates to the supplied |proc|.
  ///
  ///          This procedure will generate no more than the number of
  ///          vertices returned by |GetCircleVertexCount| in an order
  ///          appropriate for rendering as a triangle strip.
  void GenerateCircleTriangleStrip(const TessellatedPointProc& proc,
                                   const Point& center,
                                   Scalar radius) const;

  /// @brief   Generate the vertices for a triangle strip that covers the
  ///          line from |p0| to |p1| with round caps of the specified
  ///          |radius|, delivering the computed coordinates to the supplied
  ///          |proc|.
  ///
  ///          This procedure will generate no more than the number of
  ///          vertices returned by |GetCircleVertexCount| in an order
  ///          appropriate for rendering as a triangle strip.
  void GenerateRoundCapLineTriangleStrip(const TessellatedPointProc& proc,
                                         const Point& p0,
                                         const Point& p1,
                                         Scalar radius) const;

 private:
  const size_t quadrant_divisions_;

  /// @brief   Constructs a CircleDivider that produces enough segments to
  ///          reasonably approximate a circle with a specified radius
  ///          in pixels.
  constexpr explicit CircleTessellator(Scalar pixel_radius)
      : quadrant_divisions_(ComputeQuadrantDivisions(pixel_radius)) {}

  CircleTessellator(const CircleTessellator&) = delete;

  CircleTessellator& operator=(const CircleTessellator&) = delete;

  /// @brief   Compute the number of vertices to divide each quadrant of
  ///          the circle into based on the expected pixel space radius.
  ///
  /// @return  the number of vertices.
  static size_t ComputeQuadrantDivisions(Scalar pixel_radius);

  /// @brief   Compute the sine and cosine for each angle in the number of
  ///          divisions [0, divisions] of a quarter circle and return the
  ///          values in a vector of trig objects.
  ///
  ///          Note that since the 0th division is included, the vector will
  ///          contain (divisions + 1) values.
  ///
  /// @return  The vector of (divisions + 1) trig values.
  static const std::vector<Trig>& GetTrigForDivisions(size_t divisions);

  static constexpr int MAX_DIVISIONS_ = 35;

  static std::vector<Trig> trigs_[MAX_DIVISIONS_ + 1];
};

}  // namespace impeller
