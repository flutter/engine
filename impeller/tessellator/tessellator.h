// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "flutter/fml/macros.h"
#include "impeller/core/formats.h"
#include "impeller/geometry/path.h"
#include "impeller/geometry/point.h"
#include "impeller/geometry/trig.h"

struct TESStesselator;

namespace impeller {

void DestroyTessellator(TESStesselator* tessellator);

using CTessellator =
    std::unique_ptr<TESStesselator, decltype(&DestroyTessellator)>;

enum class WindingOrder {
  kClockwise,
  kCounterClockwise,
};

//------------------------------------------------------------------------------
/// @brief      A utility that generates triangles of the specified fill type
///             given a polyline. This happens on the CPU.
///
///             This object is not thread safe, and its methods must not be
///             called from multiple threads.
///
class Tessellator {
 public:
  enum class Result {
    kSuccess,
    kInputError,
    kTessellationError,
  };

  /// @brief  A callback function for a |VertexGenerator| to deliver
  ///         the vertices it computes as |Point| objects.
  using TessellatedVertexProc = std::function<void(const Point& p)>;

  /// @brief  An object which produces a list of vertices as |Point|s that
  ///         tessellate a previously provided shape and delivers the vertices
  ///         through a |TessellatedVertexProc| callback.
  ///
  ///         The object can also provide advance information on how many
  ///         vertices it will generate.
  ///
  /// @see |Tessellator::FilledCircle|
  /// @see |Tessellator::StrokedCircle|
  /// @see |Tessellator::RoundCapLine|
  /// @see |Tessellator::FilledEllipse|
  class VertexGenerator {
   public:
    virtual ~VertexGenerator() = default;

    /// @brief  Returns the |PrimitiveType| that describe the relationship
    ///         among the list of vertices produced by the |GenerateVertices|
    ///         method.
    ///
    ///         Most generators will deliver |kTriangleStrip| triangles
    virtual PrimitiveType GetPrimitiveType() const = 0;

    /// @brief  Returns the number of vertices that the generator plans to
    ///         produce, if known.
    ///
    ///         The return value for this method is advisory only and
    ///         can be used to reserve space where the vertices will
    ///         be placed, but the count may be an estimate.
    ///
    ///         Implementations are encouraged to avoid overestimating
    ///         the count by too large a number and to provide a best
    ///         guess so as to minimize potential buffer reallocations
    ///         as the vertices are delivered.
    virtual size_t VertexCount() const = 0;

    /// @brief  Generate the vertices and deliver them in the necessary
    ///         order (as required by the PrimitiveType) to the given
    ///         callback function.
    virtual void GenerateVertices(const TessellatedVertexProc& proc) const = 0;
  };

  Tessellator();

  ~Tessellator();

  /// @brief A callback that returns the results of the tessellation.
  ///
  ///        The index buffer may not be populated, in which case [indices] will
  ///        be nullptr and indices_count will be 0.
  using BuilderCallback = std::function<bool(const float* vertices,
                                             size_t vertices_count,
                                             const uint16_t* indices,
                                             size_t indices_count)>;

  //----------------------------------------------------------------------------
  /// @brief      Generates filled triangles from the path. A callback is
  ///             invoked once for the entire tessellation.
  ///
  /// @param[in]  path  The path to tessellate.
  /// @param[in]  tolerance  The tolerance value for conversion of the path to
  ///                        a polyline. This value is often derived from the
  ///                        Matrix::GetMaxBasisLength of the CTM applied to the
  ///                        path for rendering.
  /// @param[in]  callback  The callback, return false to indicate failure.
  ///
  /// @return The result status of the tessellation.
  ///
  Tessellator::Result Tessellate(const Path& path,
                                 Scalar tolerance,
                                 const BuilderCallback& callback);

  //----------------------------------------------------------------------------
  /// @brief      Given a convex path, create a triangle fan structure.
  ///
  /// @param[in]  path  The path to tessellate.
  /// @param[in]  tolerance  The tolerance value for conversion of the path to
  ///                        a polyline. This value is often derived from the
  ///                        Matrix::GetMaxBasisLength of the CTM applied to the
  ///                        path for rendering.
  ///
  /// @return A point vector containing the vertices in triangle strip format.
  ///
  std::vector<Point> TessellateConvex(const Path& path, Scalar tolerance);

  /// @brief   The pixel tolerance used by the algorighm to determine how
  ///          many divisions to create for a circle.
  ///
  ///          No point on the polygon of vertices should deviate from the
  ///          true circle by more than this tolerance.
  static constexpr Scalar kCircleTolerance = 0.1f;

  /// @brief   Create a |VertexGenerator| that can produce vertices for
  ///          a filled circle of the given radius around the given center
  ///          with enough polygon sub-divisions to provide reasonable
  ///          fidelity when viewed under the given view transform.
  ///
  ///          Note that the view transform is only used to choose the
  ///          number of sample points to use per quarter circle and the
  ///          returned points are not transformed by it, instead they are
  ///          relative to the coordinate space of the center point.
  std::unique_ptr<VertexGenerator> FilledCircle(const Matrix& view_transform,
                                                const Point& center,
                                                Scalar radius);

  /// @brief   Create a |VertexGenerator| that can produce vertices for
  ///          a stroked circle of the given outer and inner radii around
  ///          the given shared center with enough polygon sub-divisions
  ///          to provide reasonable fidelity when viewed under the given
  ///          view transform.
  ///
  ///          Note that the view transform is only used to choose the
  ///          number of sample points to use per quarter circle and the
  ///          returned points are not transformed by it, instead they are
  ///          relative to the coordinate space of the center point.
  std::unique_ptr<VertexGenerator> StrokedCircle(const Matrix& view_transform,
                                                 const Point& center,
                                                 Scalar outer_radius,
                                                 Scalar inner_radius);

  /// @brief   Create a |VertexGenerator| that can produce vertices for
  ///          a line with round end caps of the given radius with enough
  ///          polygon sub-divisions to provide reasonable fidelity when
  ///          viewed under the given view transform.
  ///
  ///          Note that the view transform is only used to choose the
  ///          number of sample points to use per quarter circle and the
  ///          returned points are not transformed by it, instead they are
  ///          relative to the coordinate space of the two points.
  std::unique_ptr<VertexGenerator> RoundCapLine(const Matrix& view_transform,
                                                const Point& p0,
                                                const Point& p1,
                                                Scalar radius);

  /// @brief   Create a |VertexGenerator| that can produce vertices for
  ///          a filled ellipse inscribed within the given bounds with
  ///          enough polygon sub-divisions to provide reasonable
  ///          fidelity when viewed under the given view transform.
  ///
  ///          Note that the view transform is only used to choose the
  ///          number of sample points to use per quarter circle and the
  ///          returned points are not transformed by it, instead they are
  ///          relative to the coordinate space of the bounds.
  std::unique_ptr<VertexGenerator> FilledEllipse(const Matrix& view_transform,
                                                 const Rect& bounds);

 private:
  /// Used for polyline generation.
  std::unique_ptr<std::vector<Point>> point_buffer_;
  CTessellator c_tessellator_;

  /// Essentially just a vector of Trig objects, but supports storing a
  /// reference to either a cached vector or a locally generated vector.
  /// The constructor will fill the vector with quarter circular samples
  /// for the indicated number of equal divisions if the vector is new.
  class Trigs {
   public:
    explicit Trigs(std::vector<Trig>& trigs, size_t divisions) : trigs_(trigs) {
      init(divisions);
      FML_DCHECK(trigs_.size() == divisions + 1);
    }

    explicit Trigs(size_t divisions)
        : local_storage_(std::make_unique<std::vector<Trig>>()),
          trigs_(*local_storage_) {
      init(divisions);
      FML_DCHECK(trigs_.size() == divisions + 1);
    }

    // Utility forwards of the indicated vector methods.
    auto inline size() const { return trigs_.size(); }
    auto inline begin() const { return trigs_.begin(); }
    auto inline end() const { return trigs_.end(); }

   private:
    // nullptr if a cached vector is used, otherwise the actual storage
    std::unique_ptr<std::vector<Trig>> local_storage_;

    // Whether or not a cached vector or the local storage is used, this
    // this reference will always be valid
    std::vector<Trig>& trigs_;

    // Fill the vector with the indicated number of equal divisions of
    // trigonometric values if it is empty.
    void init(size_t divisions);
  };

  // Data for variouos Circle/EllipseGenerator classes, cached per
  // Tessellator instance which is usually the foreground life of an app
  // if not longer.
  static constexpr size_t kCachedTrigCount = 300;
  std::vector<Trig> precomputed_trigs_[kCachedTrigCount];

  Trigs GetTrigsForDivisions(size_t divisions);

  // Base class for all of the generators that use a vector of |Trig|
  // values to generate their geometry.
  class TrigGeneratorBase : public VertexGenerator {
   public:
    explicit TrigGeneratorBase(Trigs&& trigs) : trigs_(std::move(trigs)) {}

    ~TrigGeneratorBase() = default;

   protected:
    const Trigs trigs_;
  };

  class FilledCircleGenerator : public TrigGeneratorBase {
   public:
    FilledCircleGenerator(Trigs&& trigs, const Point& center, Scalar radius);

    ~FilledCircleGenerator() = default;

    PrimitiveType GetPrimitiveType() const override {
      return PrimitiveType::kTriangleStrip;
    }

    size_t VertexCount() const override { return trigs_.size() * 4; }

    void GenerateVertices(const TessellatedVertexProc& proc) const override;

   private:
    const Point center_;
    const Scalar radius_;
  };

  class StrokedCircleGenerator : public TrigGeneratorBase {
   public:
    StrokedCircleGenerator(Trigs&& trigs,
                           const Point& center,
                           Scalar outer_radius,
                           Scalar inner_radius);

    ~StrokedCircleGenerator() = default;

    PrimitiveType GetPrimitiveType() const override {
      return PrimitiveType::kTriangleStrip;
    }

    size_t VertexCount() const override { return trigs_.size() * 8; }

    void GenerateVertices(const TessellatedVertexProc& proc) const override;

   private:
    const Point center_;
    const Scalar outer_radius_;
    const Scalar inner_radius_;
  };

  class RoundCapLineGenerator : public TrigGeneratorBase {
   public:
    RoundCapLineGenerator(Trigs&& trigs,
                          const Point& p0,
                          const Point& p1,
                          Scalar radius);

    ~RoundCapLineGenerator() = default;

    PrimitiveType GetPrimitiveType() const override {
      return PrimitiveType::kTriangleStrip;
    }

    size_t VertexCount() const override { return trigs_.size() * 4; }

    void GenerateVertices(const TessellatedVertexProc& proc) const override;

   private:
    const Point p0_;
    const Point p1_;
    const Scalar radius_;
  };

  class FilledEllipseGenerator : public TrigGeneratorBase {
   public:
    FilledEllipseGenerator(Trigs&& trigs, const Rect& bounds);

    ~FilledEllipseGenerator() = default;

    PrimitiveType GetPrimitiveType() const override {
      return PrimitiveType::kTriangleStrip;
    }

    size_t VertexCount() const override { return trigs_.size() * 4; }

    void GenerateVertices(const TessellatedVertexProc& proc) const override;

   private:
    const Rect bounds_;
  };

  Tessellator(const Tessellator&) = delete;

  Tessellator& operator=(const Tessellator&) = delete;
};

}  // namespace impeller
