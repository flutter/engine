// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/gpu/render_pipeline.h"

#include "flutter/lib/gpu/shader.h"
#include "flutter/lib/ui/painting/image.h"
#include "fml/mapping.h"
#include "impeller/core/allocator.h"
#include "impeller/core/formats.h"
#include "impeller/display_list/dl_image_impeller.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {
namespace gpu {

IMPLEMENT_WRAPPERTYPEINFO(flutter_gpu, RenderPipeline);

RenderPipeline::RenderPipeline(
    fml::RefPtr<flutter::gpu::Shader> vertex_shader,
    fml::RefPtr<flutter::gpu::Shader> fragment_shader)
    : vertex_shader_(std::move(vertex_shader)),
      fragment_shader_(std::move(fragment_shader)) {}

RenderPipeline::~RenderPipeline() = default;

std::shared_ptr<impeller::Pipeline<impeller::PipelineDescriptor>>
RenderPipeline::GetOrCreatePipeline(const RenderPass& render_pass) {
  return nullptr;
}

}  // namespace gpu
}  // namespace flutter

//----------------------------------------------------------------------------
/// Exports
///

Dart_Handle InternalFlutterGpu_RenderPipeline_Initialize(
    Dart_Handle wrapper,
    flutter::gpu::Context* gpu_context,
    flutter::gpu::Shader* vertex_shader,
    flutter::gpu::Shader* fragment_shader) {
  // Lazily register the shaders synchronously if they haven't been already.
  vertex_shader->RegisterSync(*gpu_context);
  fragment_shader->RegisterSync(*gpu_context);

  auto res = fml::MakeRefCounted<flutter::gpu::RenderPipeline>(
      fml::RefPtr<flutter::gpu::Shader>(vertex_shader),  //
      fml::RefPtr<flutter::gpu::Shader>(fragment_shader));
  res->AssociateWithDartWrapper(wrapper);

  return Dart_Null();
}
