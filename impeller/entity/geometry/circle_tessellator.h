// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

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
};

/// @brief  A utility class to compute the number of divisions for a circle
///         given a transform-adjusted pixel radius and methods for generating
///         a tessellated set of triangles for a quarter or full circle.
///
///         A helper constructor is provided which can compute the device
///         pixel radius size for a geometry-space radius when viewed under
///         a specified geometry-to-device transform.
///
///         The object should be constructed with the expected radius of the
///         circle in pixels, but can then be used to generate a triangular
///         tessellation with the indicated number of divisions for any
///         radius after that. Since the coordinate space in which the
///         circle being tessellated is not necessarily device pixel space,
///         the radius supplied during tessellation might not match the
///         pixel radius supplied during construction, but the two values
///         should be related by the transform in place when the tessellated
///         triangles are rendered for maximum tessellation fidelity.
class CircleTessellator {
 public:
  /// @brief   Constructs a CircleDivider that produces enough segments to
  ///          reasonably approximate a circle with a specified radius
  ///          in pixels.
  constexpr explicit CircleTessellator(Scalar pixel_radius)
      : quadrant_divisions_(ComputeQuadrantDivisions(pixel_radius)) {}

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
  ///          tessellate a single quarter circle.
  size_t GetQuadrantVertexCount() const { return quadrant_divisions_ * 3; }

  /// @brief   Return the number of divisions computed by the algorithm for
  ///          a full circle.
  size_t GetCircleDivisionCount() const { return quadrant_divisions_ * 4; }

  /// @brief   Return the number of vertices that will be generated to
  ///          tessellate a full circle.
  size_t GetCircleVertexCount() const { return quadrant_divisions_ * 12; }

  /// @brief   Compute the points of a triangular tesselation of the full
  ///          circle of the given radius and center.
  ///
  /// @return  the list of points on the polygonal approximation.
  std::vector<Point> GetCircleTriangles(const Point& center,
                                        Scalar radius) const;

  /// @brief   Adds entries in an existing vector of |vertices| to represent
  ///          the triangular tessellation of the quarter circle that sweeps
  ///          from the |start_vector| to the |end_vector| around an origin
  ///          specified by |center|. The length of the start and end vectors
  ///          controls the size of the quarter circle.
  ///
  ///          The axes must be of the same length and perpendicular. The new
  ///          points will be appended to the end of the vector.
  void FillQuadrantTriangles(std::vector<Point>& vertices,
                             const Point& center,
                             const Point& start_vector,
                             const Point& end_vector) const;

 private:
  const size_t quadrant_divisions_;

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

  /// @brief   Extend a list of |points| in the vector containing relative
  ///          coordinates for the first quadrant (top-center to right-center)
  ///          into the remaining 3 quadrants and adjust them to be relative
  ///          to the supplied |center|.
  ///
  ///          The incoming coordinates are assumed to be relative to a
  ///          center point of (0, 0) and the method will duplicate and
  ///          reflect them around that origin to fill in the remaining
  ///          3 quadrants. As the method works, it will also adjust every
  ///          point (including the pre-existing 1st quadrant points) to
  ///          be relative to the new center.
  static void ExtendRelativeQuadrantToAbsoluteCircle(std::vector<Point>& points,
                                                     const Point& center = {});

  static constexpr int MAX_DIVISIONS_ = 35;

  static std::vector<Trig> trigs_[MAX_DIVISIONS_ + 1];
};

}  // namespace impeller
