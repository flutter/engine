// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/time/time_point.h"
#include "flutter/testing/testing.h"
#include "impeller/fixtures/impeller.frag.h"
#include "impeller/fixtures/impeller.vert.h"
#include "impeller/playground/playground.h"
#include "impeller/playground/playground_test.h"
#include "impeller/renderer/pipeline_library.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/renderer/sampler_library.h"

#include "third_party/tinygltf/tiny_gltf.h"


namespace impeller {
namespace testing {

using SceneTest = PlaygroundTest;
INSTANTIATE_PLAYGROUND_SUITE(SceneTest);

TEST_P(SceneTest, TheImpeller) {

  tinygltf::TinyGLTF ctx;

  using VS = ImpellerVertexShader;
  using FS = ImpellerFragmentShader;

  auto context = GetContext();
  auto pipeline_descriptor =
      PipelineBuilder<VS, FS>::MakeDefaultPipelineDescriptor(*context);
  ASSERT_TRUE(pipeline_descriptor.has_value());
  pipeline_descriptor->SetSampleCount(SampleCount::kCount4);
  auto pipeline =
      context->GetPipelineLibrary()->GetPipeline(pipeline_descriptor).get();
  ASSERT_TRUE(pipeline && pipeline->IsValid());

  auto blue_noise = CreateTextureForFixture("blue_noise.png");
  SamplerDescriptor noise_sampler_desc;
  noise_sampler_desc.width_address_mode = SamplerAddressMode::kRepeat;
  noise_sampler_desc.height_address_mode = SamplerAddressMode::kRepeat;
  auto noise_sampler =
      context->GetSamplerLibrary()->GetSampler(noise_sampler_desc);

  auto cube_map = CreateTextureCubeForFixture(
      {"table_mountain_px.png", "table_mountain_nx.png",
       "table_mountain_py.png", "table_mountain_ny.png",
       "table_mountain_pz.png", "table_mountain_nz.png"});
  auto cube_map_sampler = context->GetSamplerLibrary()->GetSampler({});

  SinglePassCallback callback = [&](RenderPass& pass) {
    auto size = pass.GetRenderTargetSize();

    Command cmd;
    cmd.pipeline = pipeline;
    cmd.label = "Impeller SDF scene";
    VertexBufferBuilder<VS::PerVertexData> builder;
    builder.AddVertices({{Point()},
                         {Point(0, size.height)},
                         {Point(size.width, 0)},
                         {Point(size.width, 0)},
                         {Point(0, size.height)},
                         {Point(size.width, size.height)}});
    cmd.BindVertices(builder.CreateVertexBuffer(pass.GetTransientsBuffer()));

    VS::FrameInfo vs_uniform;
    vs_uniform.mvp = Matrix::MakeOrthographic(size);
    VS::BindFrameInfo(cmd,
                      pass.GetTransientsBuffer().EmplaceUniform(vs_uniform));

    FS::FragInfo fs_uniform;
    fs_uniform.texture_size = Point(size);
    fs_uniform.time = fml::TimePoint::Now().ToEpochDelta().ToSecondsF();
    FS::BindFragInfo(cmd,
                     pass.GetTransientsBuffer().EmplaceUniform(fs_uniform));
    FS::BindBlueNoise(cmd, blue_noise, noise_sampler);
    FS::BindCubeMap(cmd, cube_map, cube_map_sampler);

    pass.AddCommand(cmd);
    return true;
  };
  OpenPlaygroundHere(callback);
}

}  // namespace testing
}  // namespace impeller
