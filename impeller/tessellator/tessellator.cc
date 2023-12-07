// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/tessellator/tessellator.h"

#include "third_party/libtess2/Include/tesselator.h"

namespace impeller {

static void* HeapAlloc(void* userData, unsigned int size) {
  return malloc(size);
}

static void* HeapRealloc(void* userData, void* ptr, unsigned int size) {
  return realloc(ptr, size);
}

static void HeapFree(void* userData, void* ptr) {
  free(ptr);
}

// Note: these units are "number of entities" for bucket size and not in KB.
static const TESSalloc kAlloc = {
    HeapAlloc, HeapRealloc, HeapFree, 0, /* =userData */
    16,                                  /* =meshEdgeBucketSize */
    16,                                  /* =meshVertexBucketSize */
    16,                                  /* =meshFaceBucketSize */
    16,                                  /* =dictNodeBucketSize */
    16,                                  /* =regionBucketSize */
    0                                    /* =extraVertices */
};

Tessellator::Tessellator()
    : point_buffer_(std::make_unique<std::vector<Point>>()),
      c_tessellator_(nullptr, &DestroyTessellator) {
  point_buffer_->reserve(2048);
  TESSalloc alloc = kAlloc;
  {
    // libTess2 copies the TESSalloc despite the non-const argument.
    CTessellator tessellator(::tessNewTess(&alloc), &DestroyTessellator);
    c_tessellator_ = std::move(tessellator);
  }
}

Tessellator::~Tessellator() = default;

static int ToTessWindingRule(FillType fill_type) {
  switch (fill_type) {
    case FillType::kOdd:
      return TESS_WINDING_ODD;
    case FillType::kNonZero:
      return TESS_WINDING_NONZERO;
    case FillType::kPositive:
      return TESS_WINDING_POSITIVE;
    case FillType::kNegative:
      return TESS_WINDING_NEGATIVE;
    case FillType::kAbsGeqTwo:
      return TESS_WINDING_ABS_GEQ_TWO;
  }
  return TESS_WINDING_ODD;
}

Tessellator::Result Tessellator::Tessellate(const Path& path,
                                            Scalar tolerance,
                                            const BuilderCallback& callback) {
  if (!callback) {
    return Result::kInputError;
  }

  point_buffer_->clear();
  auto polyline =
      path.CreatePolyline(tolerance, std::move(point_buffer_),
                          [this](Path::Polyline::PointBufferPtr point_buffer) {
                            point_buffer_ = std::move(point_buffer);
                          });

  auto fill_type = path.GetFillType();

  if (polyline.points->empty()) {
    return Result::kInputError;
  }

  auto tessellator = c_tessellator_.get();
  if (!tessellator) {
    return Result::kTessellationError;
  }

  constexpr int kVertexSize = 2;
  constexpr int kPolygonSize = 3;

  //----------------------------------------------------------------------------
  /// Feed contour information to the tessellator.
  ///
  static_assert(sizeof(Point) == 2 * sizeof(float));
  for (size_t contour_i = 0; contour_i < polyline.contours.size();
       contour_i++) {
    size_t start_point_index, end_point_index;
    std::tie(start_point_index, end_point_index) =
        polyline.GetContourPointBounds(contour_i);

    ::tessAddContour(tessellator,  // the C tessellator
                     kVertexSize,  //
                     polyline.points->data() + start_point_index,  //
                     sizeof(Point),                                //
                     end_point_index - start_point_index           //
    );
  }

  //----------------------------------------------------------------------------
  /// Let's tessellate.
  ///
  auto result = ::tessTesselate(tessellator,                   // tessellator
                                ToTessWindingRule(fill_type),  // winding
                                TESS_POLYGONS,                 // element type
                                kPolygonSize,                  // polygon size
                                kVertexSize,                   // vertex size
                                nullptr  // normal (null is automatic)
  );

  if (result != 1) {
    return Result::kTessellationError;
  }

  int element_item_count = tessGetElementCount(tessellator) * kPolygonSize;

  // We default to using a 16bit index buffer, but in cases where we generate
  // more tessellated data than this can contain we need to fall back to
  // dropping the index buffer entirely. Instead code could instead switch to
  // a uint32 index buffer, but this is done for simplicity with the other
  // fast path above.
  if (element_item_count < USHRT_MAX) {
    int vertex_item_count = tessGetVertexCount(tessellator);
    auto vertices = tessGetVertices(tessellator);
    auto elements = tessGetElements(tessellator);

    // libtess uses an int index internally due to usage of -1 as a sentinel
    // value.
    std::vector<uint16_t> indices(element_item_count);
    for (int i = 0; i < element_item_count; i++) {
      indices[i] = static_cast<uint16_t>(elements[i]);
    }
    if (!callback(vertices, vertex_item_count, indices.data(),
                  element_item_count)) {
      return Result::kInputError;
    }
  } else {
    std::vector<Point> points;
    std::vector<float> data;

    int vertex_item_count = tessGetVertexCount(tessellator) * kVertexSize;
    auto vertices = tessGetVertices(tessellator);
    points.reserve(vertex_item_count);
    for (int i = 0; i < vertex_item_count; i += 2) {
      points.emplace_back(vertices[i], vertices[i + 1]);
    }

    int element_item_count = tessGetElementCount(tessellator) * kPolygonSize;
    auto elements = tessGetElements(tessellator);
    data.reserve(element_item_count);
    for (int i = 0; i < element_item_count; i++) {
      data.emplace_back(points[elements[i]].x);
      data.emplace_back(points[elements[i]].y);
    }
    if (!callback(data.data(), element_item_count, nullptr, 0u)) {
      return Result::kInputError;
    }
  }

  return Result::kSuccess;
}

std::vector<Point> Tessellator::TessellateConvex(const Path& path,
                                                 Scalar tolerance) {
  std::vector<Point> output;

  point_buffer_->clear();
  auto polyline =
      path.CreatePolyline(tolerance, std::move(point_buffer_),
                          [this](Path::Polyline::PointBufferPtr point_buffer) {
                            point_buffer_ = std::move(point_buffer);
                          });

  output.reserve(polyline.points->size() +
                 (4 * (polyline.contours.size() - 1)));
  for (auto j = 0u; j < polyline.contours.size(); j++) {
    auto [start, end] = polyline.GetContourPointBounds(j);
    auto first_point = polyline.GetPoint(start);

    // Some polygons will not self close and an additional triangle
    // must be inserted, others will self close and we need to avoid
    // inserting an extra triangle.
    if (polyline.GetPoint(end - 1) == first_point) {
      end--;
    }

    if (j > 0) {
      // Triangle strip break.
      output.emplace_back(output.back());
      output.emplace_back(first_point);
      output.emplace_back(first_point);
    } else {
      output.emplace_back(first_point);
    }

    size_t a = start + 1;
    size_t b = end - 1;
    while (a < b) {
      output.emplace_back(polyline.GetPoint(a));
      output.emplace_back(polyline.GetPoint(b));
      a++;
      b--;
    }
    if (a == b) {
      output.emplace_back(polyline.GetPoint(a));
    }
  }
  return output;
}

void DestroyTessellator(TESStesselator* tessellator) {
  if (tessellator != nullptr) {
    ::tessDeleteTess(tessellator);
  }
}

static constexpr int kPrecomputedDivisionCount = 1024;
static int kPrecomputedDivisions[kPrecomputedDivisionCount] = {
    // clang-format off
     1,  2,  3,  4,  4,  4,  5,  5,  5,  6,  6,  6,  7,  7,  7,  7,
     8,  8,  8,  8,  8,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10,
    10, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 12, 13,
    13, 13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 17, 17, 17, 17, 17, 17, 17, 17, 17, 17, 18, 18,
    18, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19, 19, 19, 19, 19, 19,
    19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20,
    20, 20, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 26, 26, 26, 26, 26,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 28, 28, 28,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    29, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31,
    31, 31, 31, 31, 31, 31, 31, 31, 32, 32, 32, 32, 32, 32, 32, 32,
    32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 33, 33, 33,
    33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33, 33,
    33, 33, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34, 34,
    34, 34, 34, 34, 34, 34, 34, 35, 35, 35, 35, 35, 35, 35, 35, 35,
    35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 35, 36, 36,
    36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36, 36,
    36, 36, 36, 36, 36, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37,
    37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 37, 38, 38, 38, 38,
    38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38, 38,
    38, 38, 38, 38, 38, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39,
    39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 39, 40, 40,
    40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40, 40,
    40, 40, 40, 40, 40, 40, 40, 41, 41, 41, 41, 41, 41, 41, 41, 41,
    41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41, 41,
    41, 41, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42,
    42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 42, 43, 43, 43, 43,
    43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43, 43,
    43, 43, 43, 43, 43, 43, 43, 43, 44, 44, 44, 44, 44, 44, 44, 44,
    44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44, 44,
    44, 44, 44, 44, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
    45, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46,
    46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 46, 47,
    47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47,
    47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 47, 48, 48, 48,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48,
    48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 49, 49, 49, 49,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49,
    49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 49, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50,
    50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 50, 51, 51, 51, 51, 51,
    51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51,
    51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 51, 52, 52, 52, 52,
    52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52,
    52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 52, 53, 53, 53,
    53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53,
    53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 53, 54,
    54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
    54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54, 54,
    54, 54, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
    55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
    55, 55, 55, 55, 55, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
    56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56, 56,
    56, 56, 56, 56, 56, 56, 56, 56, 56, 57, 57, 57, 57, 57, 57, 57,
    // clang-format on
};

static size_t ComputeQuadrantDivisions(Scalar pixel_radius) {
  if (pixel_radius <= 0.0) {
    return 1;
  }
  int radius_index = ceil(pixel_radius);
  if (radius_index < kPrecomputedDivisionCount) {
    return kPrecomputedDivisions[radius_index];
  }

  // For a circle with N divisions per quadrant, the maximum deviation of
  // the polgyon approximation from the true circle will be at the center
  // of the base of each triangular pie slice. We can compute that distance
  // by finding the midpoint of the line of the first slice and compare
  // its distance from the center of the circle to the radius. We will aim
  // to have the length of that bisector to be within |kCircleTolerance|
  // from the radius in pixels.
  //
  // Each vertex will appear at an angle of:
  //   theta(i) = (kPi / 2) * (i / N)  // for i in [0..N]
  // with each point falling at:
  //   point(i) = r * (cos(theta), sin(theta))
  // If we consider the unit circle to simplify the calculations below then
  // we need to scale the tolerance from its absolute quantity into a unit
  // circle fraction:
  //   k = tolerance / radius
  // Using this scaled tolerance below to avoid multiplying by the radius
  // throughout all of the math, we have:
  //   first point = (1, 0)   // theta(0) == 0
  //   theta = kPi / 2 / N    // theta(1)
  //   second point = (cos(theta), sin(theta)) = (c, s)
  //   midpoint = (first + second) * 0.5 = ((1 + c)/2, s/2)
  //   |midpoint| = sqrt((1 + c)*(1 + c)/4 + s*s/4)
  //     = sqrt((1 + c + c + c*c + s*s) / 4)
  //     = sqrt((1 + 2c + 1) / 4)
  //     = sqrt((2 + 2c) / 4)
  //     = sqrt((1 + c) / 2)
  //     = cos(theta / 2)     // using half-angle cosine formula
  //   error = 1 - |midpoint| = 1 - cos(theta / 2)
  //   cos(theta/2) = 1 - error
  //   theta/2 = acos(1 - error)
  //   kPi / 2 / N / 2 = acos(1 - error)
  //   kPi / 4 / acos(1 - error) = N
  // Since we need error <= k, we want divisions >= N, so we use:
  //   N = ceil(kPi / 4 / acos(1 - k))
  //
  // Math is confirmed in https://math.stackexchange.com/a/4132095
  // (keeping in mind that we are computing quarter circle divisions here)
  // which also points out a performance optimization that is accurate
  // to within an over-estimation of 1 division would be:
  //   N = ceil(kPi / 4 / sqrt(2 * k))
  // Since we have precomputed the divisions for radii up to 1024, we can
  // afford to be more accurate using the acos formula here for larger radii.
  double k = Tessellator::kCircleTolerance / pixel_radius;
  return ceil(kPiOver4 / std::acos(1 - k));
}

void Tessellator::Trigs::init(size_t divisions) {
  if (!trigs_.empty()) {
    return;
  }

  // Either not cached yet, or we are using the temp storage...
  trigs_.reserve(divisions + 1);

  double angle_scale = kPiOver2 / divisions;

  trigs_.emplace_back(1.0, 0.0);
  for (size_t i = 1; i < divisions; i++) {
    trigs_.emplace_back(Radians(i * angle_scale));
  }
  trigs_.emplace_back(0.0, 1.0);
}

Tessellator::Trigs Tessellator::GetTrigsForDivisions(size_t divisions) {
  return divisions < Tessellator::kCachedTrigCount
             ? Trigs(precomputed_trigs_[divisions], divisions)
             : Trigs(divisions);
}

using TessellatedVertexProc = Tessellator::TessellatedVertexProc;
using VertexGenerator = Tessellator::VertexGenerator;

std::unique_ptr<VertexGenerator> Tessellator::FilledCircle(
    const Matrix& view_transform,
    Point center,
    Scalar radius) {
  auto divisions =
      ComputeQuadrantDivisions(view_transform.GetMaxBasisLength() * radius);
  return std::make_unique<FilledCircleGenerator>(
      GetTrigsForDivisions(divisions), center, radius);
}

std::unique_ptr<VertexGenerator> Tessellator::StrokedCircle(
    const Matrix& view_transform,
    Point center,
    Scalar outer_radius,
    Scalar inner_radius) {
  auto divisions = ComputeQuadrantDivisions(view_transform.GetMaxBasisLength() *
                                            outer_radius);
  if (inner_radius > 0) {
    return std::make_unique<StrokedCircleGenerator>(
        GetTrigsForDivisions(divisions), center, outer_radius, inner_radius);
  } else {
    return std::make_unique<FilledCircleGenerator>(
        GetTrigsForDivisions(divisions), center, outer_radius);
  }
}

std::unique_ptr<VertexGenerator> Tessellator::RoundCapLine(
    const Matrix& view_transform,
    Point p0,
    Point p1,
    Scalar radius) {
  auto along = p1 - p0;
  auto length = along.GetLength();
  if (length < kEhCloseEnough) {
    return FilledCircle(view_transform, p0, radius);
  }
  auto divisions =
      ComputeQuadrantDivisions(view_transform.GetMaxBasisLength() * radius);
  return std::make_unique<RoundCapLineGenerator>(
      GetTrigsForDivisions(divisions), p0, p1, radius);
}

std::unique_ptr<VertexGenerator> Tessellator::FilledEllipse(
    const Matrix& view_transform,
    Rect bounds) {
  if (bounds.GetSize().width == bounds.GetSize().height) {
    return FilledCircle(view_transform, bounds.GetCenter(),
                        bounds.GetSize().width * 0.5f);
  }
  auto max_radius = bounds.GetSize().MaxDimension();
  auto divisions =
      ComputeQuadrantDivisions(view_transform.GetMaxBasisLength() * max_radius);
  return std::make_unique<FilledEllipseGenerator>(
      GetTrigsForDivisions(divisions), bounds);
}

Tessellator::FilledCircleGenerator::FilledCircleGenerator(Trigs&& trigs,
                                                          const Point& center,
                                                          Scalar radius)
    : TrigGeneratorBase(std::move(trigs)), center_(center), radius_(radius) {}

void Tessellator::FilledCircleGenerator::GenerateVertices(
    const TessellatedVertexProc& proc) const {
  // Quadrant 1 connecting with Quadrant 4:
  for (auto& trig : trigs_) {
    auto offset = trig * radius_;
    proc({center_.x - offset.x, center_.y + offset.y});
    proc({center_.x - offset.x, center_.y - offset.y});
  }

  // The second half of the circle should be iterated in reverse, but
  // we can instead iterate forward and swap the x/y values of the
  // offset as the angles should be symmetric and thus should generate
  // symmetrically reversed trig vectors.
  // Quadrant 2 connecting with Quadrant 2:
  for (auto& trig : trigs_) {
    auto offset = trig * radius_;
    proc({center_.x + offset.y, center_.y + offset.x});
    proc({center_.x + offset.y, center_.y - offset.x});
  }
}

Tessellator::StrokedCircleGenerator::StrokedCircleGenerator(Trigs&& trigs,
                                                            const Point& center,
                                                            Scalar outer_radius,
                                                            Scalar inner_radius)
    : TrigGeneratorBase(std::move(trigs)),
      center_(center),
      outer_radius_(outer_radius),
      inner_radius_(inner_radius) {}

void Tessellator::StrokedCircleGenerator::GenerateVertices(
    const TessellatedVertexProc& proc) const {
  // Zig-zag back and forth between points on the outer circle and the
  // inner circle. Both circles are evaluated at the same number of
  // quadrant divisions so the points for a given division should match
  // 1 for 1 other than their applied radius.

  // Quadrant 1:
  for (auto& trig : trigs_) {
    auto outer = trig * outer_radius_;
    auto inner = trig * inner_radius_;
    proc({center_.x - outer.x, center_.y - outer.y});
    proc({center_.x - inner.x, center_.y - inner.y});
  }

  // The even quadrants of the circle should be iterated in reverse, but
  // we can instead iterate forward and swap the x/y values of the
  // offset as the angles should be symmetric and thus should generate
  // symmetrically reversed trig vectors.
  // Quadrant 2:
  for (auto& trig : trigs_) {
    auto outer = trig * outer_radius_;
    auto inner = trig * inner_radius_;
    proc({center_.x + outer.y, center_.y - outer.x});
    proc({center_.x + inner.y, center_.y - inner.x});
  }

  // Quadrant 3:
  for (auto& trig : trigs_) {
    auto outer = trig * outer_radius_;
    auto inner = trig * inner_radius_;
    proc({center_.x + outer.x, center_.y + outer.y});
    proc({center_.x + inner.x, center_.y + inner.y});
  }

  // Quadrant 4:
  for (auto& trig : trigs_) {
    auto outer = trig * outer_radius_;
    auto inner = trig * inner_radius_;
    proc({center_.x - outer.y, center_.y + outer.x});
    proc({center_.x - inner.y, center_.y + inner.x});
  }
}

Tessellator::RoundCapLineGenerator::RoundCapLineGenerator(Trigs&& trigs,
                                                          const Point& p0,
                                                          const Point& p1,
                                                          Scalar radius)
    : TrigGeneratorBase(std::move(trigs)), p0_(p0), p1_(p1), radius_(radius) {}

void Tessellator::RoundCapLineGenerator::GenerateVertices(
    const TessellatedVertexProc& proc) const {
  auto along = p1_ - p0_;
  along *= radius_ / along.GetLength();
  auto across = Point(-along.y, along.x);

  for (auto& trig : trigs_) {
    auto relative_along = along * trig.cos;
    auto relative_across = across * trig.sin;
    proc(p0_ - relative_along + relative_across);
    proc(p0_ - relative_along - relative_across);
  }

  // The second half of the round caps should be iterated in reverse, but
  // we can instead iterate forward and swap the sin/cos values as they
  // should be symmetric.
  for (auto& trig : trigs_) {
    auto relative_along = along * trig.sin;
    auto relative_across = across * trig.cos;
    proc(p1_ + relative_along + relative_across);
    proc(p1_ + relative_along - relative_across);
  }
}

Tessellator::FilledEllipseGenerator::FilledEllipseGenerator(Trigs&& trigs,
                                                            const Rect& bounds)
    : TrigGeneratorBase(std::move(trigs)), bounds_(bounds) {}

void Tessellator::FilledEllipseGenerator::GenerateVertices(
    const TessellatedVertexProc& proc) const {
  auto half_size = bounds_.GetSize() * 0.5f;
  auto center = bounds_.GetCenter();
  // Quadrant 1 connecting with Quadrant 4:
  for (auto& trig : trigs_) {
    auto offset = trig * half_size;
    proc({center.x - offset.x, center.y + offset.y});
    proc({center.x - offset.x, center.y - offset.y});
  }

  // The second half of the circle should be iterated in reverse, but
  // we can instead iterate forward and swap the x/y values of the
  // offset as the angles should be symmetric and thus should generate
  // symmetrically reversed trig vectors.
  // Quadrant 2 connecting with Quadrant 2:
  for (auto& trig : trigs_) {
    auto offset =
        Point(trig.sin * half_size.width, trig.cos * half_size.height);
    proc({center.x + offset.x, center.y + offset.y});
    proc({center.x + offset.x, center.y - offset.y});
  }
}

}  // namespace impeller
