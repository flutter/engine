#include "impeller/renderer/backend/gles/render_pass_utils.h"
#include "impeller/renderer/backend/gles/formats_gles.h"

namespace impeller {

void ConfigureBlending(const ProcTableGLES& gl,
                       const ColorAttachmentDescriptor* color) {
  if (color->blending_enabled) {
    gl.Enable(GL_BLEND);
    gl.BlendFuncSeparate(
        ToBlendFactor(color->src_color_blend_factor),  // src color
        ToBlendFactor(color->dst_color_blend_factor),  // dst color
        ToBlendFactor(color->src_alpha_blend_factor),  // src alpha
        ToBlendFactor(color->dst_alpha_blend_factor)   // dst alpha
    );
    gl.BlendEquationSeparate(
        ToBlendOperation(color->color_blend_op),  // mode color
        ToBlendOperation(color->alpha_blend_op)   // mode alpha
    );
  } else {
    gl.Disable(GL_BLEND);
  }

  {
    const auto is_set = [](ColorWriteMask mask,
                           ColorWriteMask check) -> GLboolean {
      return (mask & check) ? GL_TRUE : GL_FALSE;
    };

    gl.ColorMask(
        is_set(color->write_mask, ColorWriteMaskBits::kRed),    // red
        is_set(color->write_mask, ColorWriteMaskBits::kGreen),  // green
        is_set(color->write_mask, ColorWriteMaskBits::kBlue),   // blue
        is_set(color->write_mask, ColorWriteMaskBits::kAlpha)   // alpha
    );
  }
}
void ConfigureStencil(GLenum face,
                      const ProcTableGLES& gl,
                      const StencilAttachmentDescriptor& stencil,
                      uint32_t stencil_reference) {
  gl.StencilOpSeparate(
      face,                                    // face
      ToStencilOp(stencil.stencil_failure),    // stencil fail
      ToStencilOp(stencil.depth_failure),      // depth fail
      ToStencilOp(stencil.depth_stencil_pass)  // depth stencil pass
  );
  gl.StencilFuncSeparate(face,                                        // face
                         ToCompareFunction(stencil.stencil_compare),  // func
                         stencil_reference,                           // ref
                         stencil.read_mask                            // mask
  );
  gl.StencilMaskSeparate(face, stencil.write_mask);
}

void ConfigureStencil(const ProcTableGLES& gl,
                      const PipelineDescriptor& pipeline,
                      uint32_t stencil_reference) {
  if (!pipeline.HasStencilAttachmentDescriptors()) {
    gl.Disable(GL_STENCIL_TEST);
    return;
  }

  gl.Enable(GL_STENCIL_TEST);
  const auto& front = pipeline.GetFrontStencilAttachmentDescriptor();
  const auto& back = pipeline.GetBackStencilAttachmentDescriptor();

  if (front.has_value() && back.has_value() && front == back) {
    ConfigureStencil(GL_FRONT_AND_BACK, gl, *front, stencil_reference);
    return;
  }
  if (front.has_value()) {
    ConfigureStencil(GL_FRONT, gl, *front, stencil_reference);
  }
  if (back.has_value()) {
    ConfigureStencil(GL_BACK, gl, *back, stencil_reference);
  }
}

bool BindVertexBuffer(const ProcTableGLES& gl,
                      BufferBindingsGLES* vertex_desc_gles,
                      const BufferView& vertex_buffer_view,
                      size_t buffer_index) {
  if (!vertex_buffer_view) {
    return false;
  }

  const DeviceBuffer* vertex_buffer = vertex_buffer_view.GetBuffer();

  if (!vertex_buffer) {
    return false;
  }

  const auto& vertex_buffer_gles = DeviceBufferGLES::Cast(*vertex_buffer);
  if (!vertex_buffer_gles.BindAndUploadDataIfNecessary(
          DeviceBufferGLES::BindingType::kArrayBuffer)) {
    return false;
  }

  //--------------------------------------------------------------------------
  /// Bind the vertex attributes associated with vertex buffer.
  ///
  if (!vertex_desc_gles->BindVertexAttributes(
          gl, buffer_index, vertex_buffer_view.GetRange().offset)) {
    return false;
  }

  return true;
}

}  // namespace impeller