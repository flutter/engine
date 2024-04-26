// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/wasm/box_scene.h"

#include "impeller/fixtures/box_fade.frag.h"
#include "impeller/fixtures/box_fade.vert.h"
#include "impeller/renderer/pipeline_library.h"

namespace impeller::wasm {

using VS = BoxFadeVertexShader;
using FS = BoxFadeFragmentShader;

BoxScene::BoxScene() = default;

BoxScene::~BoxScene() = default;

// |Scene|
bool BoxScene::Setup(const Context& context) {
  using BoxPipelineBuilder = PipelineBuilder<VS, FS>;
  auto desc = BoxPipelineBuilder::MakeDefaultPipelineDescriptor(context);
  FML_CHECK(desc.has_value());
  desc->SetSampleCount(SampleCount::kCount1);
  desc->SetStencilAttachmentDescriptors(std::nullopt);
  auto pipeline = context.GetPipelineLibrary()->GetPipeline(desc).Get();
  FML_CHECK(pipeline && pipeline->IsValid());

  // // Vertex buffer.
  // VertexBufferBuilder<VS::PerVertexData> vertex_builder;
  // vertex_builder.SetLabel("Box");
  // vertex_builder.AddVertices({
  //     {{100, 100, 0.0}, {0.0, 0.0}},  // 1
  //     {{800, 100, 0.0}, {1.0, 0.0}},  // 2
  //     {{800, 800, 0.0}, {1.0, 1.0}},  // 3
  //     {{100, 100, 0.0}, {0.0, 0.0}},  // 1
  //     {{800, 800, 0.0}, {1.0, 1.0}},  // 3
  //     {{100, 800, 0.0}, {0.0, 1.0}},  // 4
  // });
  // auto bridge = CreateTextureForFixture("bay_bridge.jpg");
  // auto boston = CreateTextureForFixture("boston.jpg");
  // ASSERT_TRUE(bridge && boston);
  // const std::unique_ptr<const Sampler>& sampler =
  //     context->GetSamplerLibrary()->GetSampler({});
  // ASSERT_TRUE(sampler);
  return true;
}

// |Scene|
bool BoxScene::Render(const Context& context, RenderPass& pass) {
  return true;
  // pass.SetCommandLabel("Box");
  // pass.SetPipeline(pipeline);
  // pass.SetVertexBuffer(
  //     vertex_builder.CreateVertexBuffer(*context->GetResourceAllocator()));

  // VS::UniformBuffer uniforms;
  // EXPECT_EQ(pass.GetOrthographicTransform(),
  //           Matrix::MakeOrthographic(pass.GetRenderTargetSize()));
  // uniforms.mvp =
  //     pass.GetOrthographicTransform() * Matrix::MakeScale(GetContentScale());
  // VS::BindUniformBuffer(pass, host_buffer->EmplaceUniform(uniforms));

  // FS::FrameInfo frame_info;
  // frame_info.current_time = GetSecondsElapsed();
  // frame_info.cursor_position = GetCursorPosition();
  // frame_info.window_size.x = GetWindowSize().width;
  // frame_info.window_size.y = GetWindowSize().height;

  // FS::BindFrameInfo(pass, host_buffer->EmplaceUniform(frame_info));
  // FS::BindContents1(pass, boston, sampler);
  // FS::BindContents2(pass, bridge, sampler);

  // host_buffer->Reset();
  // return pass.Draw().ok();
}

// |Scene|
bool BoxScene::Teardown(const Context& context) {
  return true;
}

}  // namespace impeller::wasm
