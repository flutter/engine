// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_GLES_RENDER_PASS_UTILS_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_GLES_RENDER_PASS_UTILS_H_

#include "impeller/renderer/backend/gles/buffer_bindings_gles.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"
#include "impeller/renderer/pipeline_descriptor.h"

namespace impeller {

//------------------------------------------------------------------------------
/// @brief      Encapsulates data that will be needed in the reactor for the
///             encoding of commands for this render pass.
///
struct RenderPassData {
  Viewport viewport;

  Color clear_color;
  uint32_t clear_stencil = 0u;
  Scalar clear_depth = 1.0;

  std::shared_ptr<Texture> color_attachment;
  std::shared_ptr<Texture> depth_attachment;
  std::shared_ptr<Texture> stencil_attachment;

  bool clear_color_attachment = true;
  bool clear_depth_attachment = true;
  bool clear_stencil_attachment = true;

  bool discard_color_attachment = true;
  bool discard_depth_attachment = true;
  bool discard_stencil_attachment = true;

  std::string label;
};

void ConfigureBlending(const ProcTableGLES& gl,
                       const ColorAttachmentDescriptor* color);

void ConfigureStencil(GLenum face,
                      const ProcTableGLES& gl,
                      const StencilAttachmentDescriptor& stencil,
                      uint32_t stencil_reference);

void ConfigureStencil(const ProcTableGLES& gl,
                      const PipelineDescriptor& pipeline,
                      uint32_t stencil_reference);

bool BindVertexBuffer(const ProcTableGLES& gl,
                      BufferBindingsGLES* vertex_desc_gles,
                      const BufferView& vertex_buffer_view,
                      size_t buffer_index);

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_GLES_RENDER_PASS_UTILS_H_
