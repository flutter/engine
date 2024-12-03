// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/pipeline_gles.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"

namespace impeller {

PipelineGLES::PipelineGLES(ReactorGLES::Ref reactor,
                           std::weak_ptr<PipelineLibrary> library,
                           const PipelineDescriptor& desc,
                           GLint program_handle)
    : Pipeline(std::move(library), desc),
      reactor_(std::move(reactor)),
      program_handle_(program_handle),
      is_valid_(program_handle != GL_NONE) {
  reactor_->SetDirectDebugLabel(program_handle_, DebugResourceType::kProgram,
                                GetDescriptor().GetLabel());
}

// |Pipeline|
PipelineGLES::~PipelineGLES() = default;

// |Pipeline|
bool PipelineGLES::IsValid() const {
  return is_valid_;
}

GLint PipelineGLES::GetProgramHandle() const {
  return program_handle_;
}

BufferBindingsGLES* PipelineGLES::GetBufferBindings() const {
  return buffer_bindings_.get();
}

bool PipelineGLES::BuildVertexDescriptor(const ProcTableGLES& gl,
                                         GLuint program) {
  if (buffer_bindings_) {
    return false;
  }
  auto vtx_desc = std::make_unique<BufferBindingsGLES>();
  if (!vtx_desc->RegisterVertexStageInput(
          gl, GetDescriptor().GetVertexDescriptor()->GetStageInputs(),
          GetDescriptor().GetVertexDescriptor()->GetStageLayouts())) {
    return false;
  }
  if (!vtx_desc->ReadUniformsBindings(gl, program)) {
    return false;
  }
  buffer_bindings_ = std::move(vtx_desc);
  return true;
}

[[nodiscard]] bool PipelineGLES::BindProgram() const {
  if (program_handle_ == GL_NONE) {
    return false;
  }
  reactor_->GetProcTable().UseProgram(program_handle_);
  return true;
}

[[nodiscard]] bool PipelineGLES::UnbindProgram() const {
  if (reactor_) {
    reactor_->GetProcTable().UseProgram(0u);
  }
  return true;
}

}  // namespace impeller
