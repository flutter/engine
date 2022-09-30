// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/tessellator/tessellator.h"

#include "third_party/libtess2/Include/tesselator.h"

namespace impeller {

static void DestroyTessellator(TESStesselator* tessellator) {
  if (tessellator != nullptr) {
    ::tessDeleteTess(tessellator);
  }
}

using CTessellator =
    std::unique_ptr<TESStesselator, decltype(&DestroyTessellator)>;

Tessellator::Tessellator() = default;

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

class TessellatorContext {
 public:
  TessellatorContext(CTessellator cTessellator)
      : cTessellator_(std::move(cTessellator)) {}

  ~TessellatorContext() { cTessellator_.reset(); }

  TESStesselator* tessellator() { return cTessellator_.get(); }

 private:
  CTessellator cTessellator_;
};

Tessellator::Result Tessellator::Tessellate(
    std::shared_ptr<TessellatorContext> context,
    FillType fill_type,
    const Path::Polyline& polyline,
    const VertexCallback& callback) const {
  if (!callback) {
    return Result::kInputError;
  }

  if (polyline.points.empty()) {
    return Result::kInputError;
  }

  auto tessellator = context->tessellator();
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
                     polyline.points.data() + start_point_index,  //
                     sizeof(Point),                               //
                     end_point_index - start_point_index          //
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

  // TODO(csg): This copy can be elided entirely for the current use case.
  std::vector<Point> points;
  std::vector<uint32_t> indices;

  int vertexItemCount = tessGetVertexCount(tessellator) * kVertexSize;
  auto vertices = tessGetVertices(tessellator);
  for (int i = 0; i < vertexItemCount; i += 2) {
    points.emplace_back(vertices[i], vertices[i + 1]);
  }

  int elementItemCount = tessGetElementCount(tessellator) * kPolygonSize;
  auto elements = tessGetElements(tessellator);
  for (int i = 0; i < elementItemCount; i++) {
    indices.emplace_back(elements[i]);
  }

  for (auto index : indices) {
    auto vtx = points[index];
    callback(vtx);
  }
  return Result::kSuccess;
}

static void* heapAlloc(void* userData, unsigned int size) {
  return malloc(size);
}

static void* heapRealloc(void* userData, void* ptr, unsigned int size) {
  return realloc(ptr, size);
}

static void heapFree(void* userData, void* ptr) {
  free(ptr);
}

static TESSalloc alloc = {
    heapAlloc, heapRealloc, heapFree, 0, /* =userData */
    16,                                  /* =meshEdgeBucketSize */
    16,                                  /* =meshVertexBucketSize */
    16,                                  /* =meshFaceBucketSize */
    16,                                  /* =dictNodeBucketSize */
    16,                                  /* =regionBucketSize */
    0                                    /* =extraVertices */
};

// static

std::shared_ptr<TessellatorContext> Tessellator::CreateTessellatorContext() {
  CTessellator tessellator(::tessNewTess(&alloc), DestroyTessellator);

  return std::make_shared<TessellatorContext>(std::move(tessellator));
}

}  // namespace impeller
