// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/gles/context_gles.h"
#include <memory>

#include "impeller/base/config.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/gles/command_buffer_gles.h"
#include "impeller/renderer/backend/gles/gpu_tracer_gles.h"
#include "impeller/renderer/backend/gles/handle_gles.h"
#include "impeller/renderer/command_queue.h"

namespace impeller {

std::shared_ptr<ContextGLES> ContextGLES::Create(
    std::unique_ptr<ProcTableGLES> gl,
    const std::vector<std::shared_ptr<fml::Mapping>>& shader_libraries,
    bool enable_gpu_tracing) {
  return std::shared_ptr<ContextGLES>(
      new ContextGLES(std::move(gl), shader_libraries, enable_gpu_tracing));
}

ContextGLES::ContextGLES(
    std::unique_ptr<ProcTableGLES> gl,
    const std::vector<std::shared_ptr<fml::Mapping>>& shader_libraries_mappings,
    bool enable_gpu_tracing) {
  reactor_ = std::make_shared<ReactorGLES>(std::move(gl));
  if (!reactor_->IsValid()) {
    VALIDATION_LOG << "Could not create valid reactor.";
    return;
  }

  // Create the shader library.
  {
    auto library = std::shared_ptr<ShaderLibraryGLES>(
        new ShaderLibraryGLES(shader_libraries_mappings));
    if (!library->IsValid()) {
      VALIDATION_LOG << "Could not create valid shader library.";
      return;
    }
    shader_library_ = std::move(library);
  }

  // Create the pipeline library.
  {
    pipeline_library_ =
        std::shared_ptr<PipelineLibraryGLES>(new PipelineLibraryGLES(reactor_));
  }

  // Create allocators.
  {
    resource_allocator_ =
        std::shared_ptr<AllocatorGLES>(new AllocatorGLES(reactor_));
    if (!resource_allocator_->IsValid()) {
      VALIDATION_LOG << "Could not create a resource allocator.";
      return;
    }
  }

  device_capabilities_ = reactor_->GetProcTable().GetCapabilities();

  // Create the sampler library.
  {
    sampler_library_ =
        std::shared_ptr<SamplerLibraryGLES>(new SamplerLibraryGLES(
            device_capabilities_->SupportsDecalSamplerAddressMode()));
  }
  gpu_tracer_ = std::make_shared<GPUTracerGLES>(GetReactor()->GetProcTable(),
                                                enable_gpu_tracing);
  command_queue_ = std::make_shared<CommandQueue>();
  is_valid_ = true;
}

ContextGLES::~ContextGLES() = default;

Context::BackendType ContextGLES::GetBackendType() const {
  return Context::BackendType::kOpenGLES;
}

const ReactorGLES::Ref& ContextGLES::GetReactor() const {
  return reactor_;
}

std::optional<ReactorGLES::WorkerID> ContextGLES::AddReactorWorker(
    const std::shared_ptr<ReactorGLES::Worker>& worker) {
  if (!IsValid()) {
    return std::nullopt;
  }
  return reactor_->AddWorker(worker);
}

bool ContextGLES::RemoveReactorWorker(ReactorGLES::WorkerID id) {
  if (!IsValid()) {
    return false;
  }
  return reactor_->RemoveWorker(id);
}

bool ContextGLES::IsValid() const {
  return is_valid_;
}

void ContextGLES::Shutdown() {}

// |Context|
std::string ContextGLES::DescribeGpuModel() const {
  return reactor_->GetProcTable().GetDescription()->GetString();
}

// |Context|
std::shared_ptr<Allocator> ContextGLES::GetResourceAllocator() const {
  return resource_allocator_;
}

// |Context|
std::shared_ptr<ShaderLibrary> ContextGLES::GetShaderLibrary() const {
  return shader_library_;
}

// |Context|
std::shared_ptr<SamplerLibrary> ContextGLES::GetSamplerLibrary() const {
  return sampler_library_;
}

// |Context|
std::shared_ptr<PipelineLibrary> ContextGLES::GetPipelineLibrary() const {
  return pipeline_library_;
}

// |Context|
std::shared_ptr<CommandBuffer> ContextGLES::CreateCommandBuffer() const {
  return std::shared_ptr<CommandBufferGLES>(
      new CommandBufferGLES(weak_from_this(), reactor_));
}

// |Context|
const std::shared_ptr<const Capabilities>& ContextGLES::GetCapabilities()
    const {
  return device_capabilities_;
}

// |Context|
std::shared_ptr<CommandQueue> ContextGLES::GetCommandQueue() const {
  return command_queue_;
}

namespace {
static const std::string_view kVertexShaderSource =
    "#version 100\n"
    "attribute vec2 position;\n"
    "attribute vec2 in_texcoord;\n"
    "varying vec2 texcoord;\n"
    "\n"
    "void main() {\n"
    "  gl_Position = vec4(position, 0, 1);\n"
    "  texcoord = in_texcoord;\n"
    "}\n";

static const std::string_view kFragmentShaderSource =
    "#version 100\n"
    "precision mediump float;\n"
    "\n"
    "uniform sampler2D texture;\n"
    "varying vec2 texcoord;\n"
    "\n"
    "void main() {\n"
    "  gl_FragColor = texture2D(texture, texcoord);\n"
    "}\n";
}  // namespace

// |Context|
std::optional<HandleGLES> ContextGLES::GetEmulatedBlitProgram() const {
  if (emulated_blit_program_.has_value()) {
    return emulated_blit_program_;
  }

  const auto& gl = reactor_->GetProcTable();
  auto vert_shader = gl.CreateShader(GL_VERTEX_SHADER);
  auto frag_shader = gl.CreateShader(GL_FRAGMENT_SHADER);

  if (vert_shader == 0 || frag_shader == 0) {
    VALIDATION_LOG << "Could not create shader handles.";
    return std::nullopt;
  }

  gl.SetDebugLabel(DebugResourceType::kShader, vert_shader,
                   SPrintF("Blit Vertex Shader"));
  gl.SetDebugLabel(DebugResourceType::kShader, frag_shader,
                   SPrintF("Blit Fragment Shader"));

  fml::ScopedCleanupClosure delete_vert_shader(
      [&gl, vert_shader]() { gl.DeleteShader(vert_shader); });
  fml::ScopedCleanupClosure delete_frag_shader(
      [&gl, frag_shader]() { gl.DeleteShader(frag_shader); });

  {
    const GLchar* sources[] = {kVertexShaderSource.data()};
    const GLint lengths[] = {static_cast<GLint>(kVertexShaderSource.size())};

    gl.ShaderSource(vert_shader, 1u, sources, lengths);
  }
  {
    const GLchar* sources[] = {kFragmentShaderSource.data()};
    const GLint lengths[] = {static_cast<GLint>(kFragmentShaderSource.size())};

    gl.ShaderSource(frag_shader, 1u, sources, lengths);
  }

  gl.CompileShader(vert_shader);
  gl.CompileShader(frag_shader);

  GLint vert_status = GL_FALSE;
  GLint frag_status = GL_FALSE;

  gl.GetShaderiv(vert_shader, GL_COMPILE_STATUS, &vert_status);
  gl.GetShaderiv(frag_shader, GL_COMPILE_STATUS, &frag_status);

  if (vert_status != GL_TRUE) {
    VALIDATION_LOG << "Failed to compile blit framebuffer emulation shader.";
    return std::nullopt;
  }

  if (frag_status != GL_TRUE) {
    VALIDATION_LOG << "Failed to compile blit framebuffer emulation shader.";
    return std::nullopt;
  }

  HandleGLES program_handle = reactor_->CreateHandle(HandleType::kProgram);
  if (program_handle.IsDead()) {
    return std::nullopt;
  }
  GLint program = program_handle.name->id;

  gl.AttachShader(program, vert_shader);
  gl.AttachShader(program, frag_shader);

  fml::ScopedCleanupClosure detach_vert_shader(
      [&gl, program = program, vert_shader]() {
        gl.DetachShader(program, vert_shader);
      });
  fml::ScopedCleanupClosure detach_frag_shader(
      [&gl, program = program, frag_shader]() {
        gl.DetachShader(program, frag_shader);
      });

  gl.BindAttribLocation(program,                 //
                        static_cast<GLuint>(0),  //
                        "position"               //
  );
  gl.BindAttribLocation(program,                 //
                        static_cast<GLuint>(1),  //
                        "in_texcoord"            //
  );

  gl.LinkProgram(program);

  GLint link_status = GL_FALSE;
  gl.GetProgramiv(program, GL_LINK_STATUS, &link_status);

  if (link_status != GL_TRUE) {
    VALIDATION_LOG << "Could not link shader program: "
                   << gl.GetProgramInfoLogString(program);
    return std::nullopt;
  }
  emulated_blit_program_ = program_handle;
  return emulated_blit_program_;
}

}  // namespace impeller
