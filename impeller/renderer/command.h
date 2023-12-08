// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>

#include "impeller/core/buffer_view.h"
#include "impeller/core/formats.h"
#include "impeller/core/sampler.h"
#include "impeller/core/shader_types.h"
#include "impeller/core/texture.h"
#include "impeller/core/vertex_buffer.h"
#include "impeller/geometry/rect.h"
#include "impeller/renderer/pipeline.h"

namespace impeller {

#ifdef IMPELLER_DEBUG
#define DEBUG_COMMAND_INFO(obj, arg) obj.label = arg;
#else
#define DEBUG_COMMAND_INFO(obj, arg)
#endif  // IMPELLER_DEBUG

struct BindingOffsets {
  size_t offset;
  size_t length;
};

//------------------------------------------------------------------------------
/// @brief      An object used to specify work to the GPU along with references
///             to resources the GPU will used when doing said work.
///
///             To construct a valid command, follow these steps:
///             * Specify a valid pipeline.
///             * Specify vertex information via a call `BindVertices`
///             * Specify any stage bindings when adding to a RenderPass.
///             * (Optional) Specify a debug label.
///
///             Command can be created frequently and on demand. The resources
///             referenced in commands views into buffers managed by other
///             allocators and resource managers.
///
struct Command {
  //----------------------------------------------------------------------------
  /// The pipeline to use for this command.
  ///
  std::shared_ptr<Pipeline<PipelineDescriptor>> pipeline;
  //----------------------------------------------------------------------------
  /// The buffer, texture, and sampler bindings used by the vertex pipeline
  /// stage.
  ///
  BindingOffsets buffer_bindings;
  //----------------------------------------------------------------------------
  /// The buffer, texture, and sampler bindings used by the fragment pipeline
  /// stage.
  ///
  BindingOffsets texture_bindings;

#ifdef IMPELLER_DEBUG
  //----------------------------------------------------------------------------
  /// The debugging label to use for the command.
  ///
  std::string label;
#endif  // IMPELLER_DEBUG

  //----------------------------------------------------------------------------
  /// The reference value to use in stenciling operations. Stencil configuration
  /// is part of pipeline setup and can be read from the pipelines descriptor.
  ///
  /// @see         `Pipeline`
  /// @see         `PipelineDescriptor`
  ///
  uint32_t stencil_reference = 0u;
  //----------------------------------------------------------------------------
  /// The offset used when indexing into the vertex buffer.
  ///
  uint64_t base_vertex = 0u;
  //----------------------------------------------------------------------------
  /// The viewport coordinates that the rasterizer linearly maps normalized
  /// device coordinates to.
  /// If unset, the viewport is the size of the render target with a zero
  /// origin, znear=0, and zfar=1.
  ///
  std::optional<Viewport> viewport;
  //----------------------------------------------------------------------------
  /// The scissor rect to use for clipping writes to the render target. The
  /// scissor rect must lie entirely within the render target.
  /// If unset, no scissor is applied.
  ///
  std::optional<IRect> scissor;
  //----------------------------------------------------------------------------
  /// The number of instances of the given set of vertices to render. Not all
  /// backends support rendering more than one instance at a time.
  ///
  /// @warning      Setting this to more than one will limit the availability of
  ///               backends to use with this command.
  ///
  size_t instance_count = 1u;

  //----------------------------------------------------------------------------
  /// The bound per-vertex data and optional index buffer.
  VertexBuffer vertex_buffer;

  //----------------------------------------------------------------------------
  /// @brief      Specify the vertex and index buffer to use for this command.
  ///
  /// @param[in]  buffer  The vertex and index buffer definition. If possible,
  ///             this value should be moved and not copied.
  ///
  /// @return     returns if the binding was updated.
  ///
  bool BindVertices(VertexBuffer buffer);

  bool IsValid() const { return pipeline && pipeline->IsValid(); }
};

}  // namespace impeller
