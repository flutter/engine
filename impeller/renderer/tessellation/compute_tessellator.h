// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/entity/geometry.h"
#include "impeller/geometry/path.h"
#include "impeller/renderer/buffer_view.h"
#include "impeller/renderer/command_buffer.h"
#include "impeller/renderer/context.h"

namespace impeller {

//------------------------------------------------------------------------------
/// @brief      A utility that generates triangles of the specified fill type
///             given a polyline. This happens on the CPU.
///
class ComputeTessellator {
 public:
  ComputeTessellator();

  ~ComputeTessellator();

  static constexpr size_t kMaxCubicCount = 256;
  static constexpr size_t kMaxQuadCount = 512;
  static constexpr size_t kMaxLineCount = 1024;
  static constexpr size_t kMaxComponentCount =
      kMaxCubicCount + kMaxQuadCount + kMaxLineCount;

  enum class Status {
    kCommandInvalid,
    kTooManyComponents,
    kOk,
  };

  enum class Style {
    kStroke,
    // TODO(dnfield): Implement kFill.
  };

  ComputeTessellator& SetStyle(Style value);
  ComputeTessellator& SetStrokeWidth(Scalar value);
  ComputeTessellator& SetStrokeJoin(Join value);
  ComputeTessellator& SetStrokeCap(Cap value);
  ComputeTessellator& SetMiterLimit(Scalar value);
  ComputeTessellator& SetCubicAccuracy(Scalar value);
  ComputeTessellator& SetQuadraticTolerance(Scalar value);

  //----------------------------------------------------------------------------
  /// @brief      Generates triangles from the path.
  ///
  /// @return  A buffer view that will be filled with the tessellation result.
  ///
  Status Tessellate(
      const Path& path,
      std::shared_ptr<Context> context,
      BufferView vertex_buffer,
      BufferView vertex_buffer_count,
      const CommandBuffer::CompletionCallback& callback = nullptr) const;

 private:
  Style style_ = Style::kStroke;
  Scalar stroke_width_ = 1.0f;
  Cap stroke_cap_ = Cap::kButt;
  Join stroke_join_ = Join::kMiter;
  Scalar miter_limit_ = 4.0f;
  Scalar cubic_accuracy_ = kDefaultCurveTolerance;
  Scalar quad_tolerance_ = .1f;

  FML_DISALLOW_COPY_AND_ASSIGN(ComputeTessellator);
};

}  // namespace impeller
