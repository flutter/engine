// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/wasm/impeller_scene.h"

#include "flutter/fml/logging.h"
#include "impeller/core/host_buffer.h"
#include "impeller/fixtures/impeller.frag.h"
#include "impeller/fixtures/impeller.vert.h"
#include "impeller/renderer/pipeline_library.h"
#include "impeller/renderer/vertex_buffer_builder.h"
#include "impeller/toolkit/wasm/fixtures_store.h"

namespace impeller::wasm {

using VS = ImpellerVertexShader;
using FS = ImpellerFragmentShader;

ImpellerScene::ImpellerScene() = default;

ImpellerScene::~ImpellerScene() = default;

bool ImpellerScene::Setup(const Context& context) {
  auto pipeline_descriptor =
      PipelineBuilder<VS, FS>::MakeDefaultPipelineDescriptor(context);
  FML_CHECK(pipeline_descriptor.has_value());
  pipeline_descriptor->SetSampleCount(SampleCount::kCount4);
  pipeline_descriptor->SetStencilAttachmentDescriptors(std::nullopt);
  auto pipeline =
      context.GetPipelineLibrary()->GetPipeline(pipeline_descriptor).Get();
  FML_CHECK(pipeline && pipeline->IsValid());
  pipeline_ = pipeline;
  blue_noise_ = CreateTextureForFixture("blue_noise.png");
  cube_map_ = CreateTextureCubeForFixture(
      {"table_mountain_px.png", "table_mountain_nx.png",
       "table_mountain_py.png", "table_mountain_ny.png",
       "table_mountain_pz.png", "table_mountain_nz.png"});
  FML_CHECK(blue_noise_ && cube_map_);
  return true;
}

bool ImpellerScene::Render(const Context& context, RenderPass& pass) {
  SamplerDescriptor noise_sampler_desc;
  noise_sampler_desc.width_address_mode = SamplerAddressMode::kRepeat;
  noise_sampler_desc.height_address_mode = SamplerAddressMode::kRepeat;
  const std::unique_ptr<const Sampler>& noise_sampler =
      context.GetSamplerLibrary()->GetSampler(noise_sampler_desc);

  const std::unique_ptr<const Sampler>& cube_map_sampler =
      context.GetSamplerLibrary()->GetSampler({});

  auto size = pass.GetRenderTargetSize();

  auto host_buffer = HostBuffer::Create(context.GetResourceAllocator());

  pass.SetPipeline(pipeline_);
  pass.SetCommandLabel("Impeller SDF scene");
  VertexBufferBuilder<VS::PerVertexData> builder;
  builder.AddVertices({{Point()},
                       {Point(0, size.height)},
                       {Point(size.width, 0)},
                       {Point(size.width, 0)},
                       {Point(0, size.height)},
                       {Point(size.width, size.height)}});
  pass.SetVertexBuffer(builder.CreateVertexBuffer(*host_buffer));

  VS::FrameInfo frame_info;
  FML_CHECK(pass.GetOrthographicTransform() == Matrix::MakeOrthographic(size));
  frame_info.mvp = pass.GetOrthographicTransform();
  VS::BindFrameInfo(pass, host_buffer->EmplaceUniform(frame_info));

  FS::FragInfo fs_uniform;
  fs_uniform.texture_size = Point(size);
  fs_uniform.time = GetSecondsElapsed();
  FS::BindFragInfo(pass, host_buffer->EmplaceUniform(fs_uniform));
  FS::BindBlueNoise(pass, blue_noise_, noise_sampler);
  FS::BindCubeMap(pass, cube_map_, cube_map_sampler);

  return pass.Draw().ok();
}

bool ImpellerScene::Teardown(const Context& context) {
  return true;
}

Scalar ImpellerScene::GetSecondsElapsed() const {
  return (fml::TimePoint::Now() - start_time_).ToSecondsF();
}

std::shared_ptr<Texture> ImpellerScene::CreateTextureCubeForFixture(
    std::array<const char*, 6> fixture_names) const {
  return nullptr;
}

std::shared_ptr<Texture> ImpellerScene::CreateTextureForFixture(
    const char* fixture) const {
  std::shared_ptr<fml::Mapping> mapping =
      GetDefaultStore()->GetMapping(fixture);
  FML_CHECK(mapping);
  return nullptr;
  // auto result = Playground::CreateTextureForMapping(GetContext(), mapping,
  //                                                   enable_mipmapping);
  // if (result) {
  //   result->SetLabel(fixture_name);
  // }
  // return result;
}

}  // namespace impeller::wasm
