// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/vertex_buffer_builder.h"
#include "lib/gpu/texture.h"
#include "testing/test_dart_native_resolver.h"
#define FML_USED_ON_EMBEDDER

#include <memory>

#include "flutter/common/settings.h"
#include "flutter/common/task_runners.h"
#include "flutter/lib/gpu/context.h"
#include "flutter/lib/gpu/shader_library.h"
#include "flutter/runtime/dart_isolate.h"
#include "flutter/runtime/dart_vm_lifecycle.h"
#include "flutter/testing/dart_fixture.h"
#include "flutter/testing/dart_isolate_runner.h"
#include "flutter/testing/testing.h"
#include "fml/memory/ref_ptr.h"
#include "impeller/fixtures/texture.frag.h"
#include "impeller/fixtures/texture.vert.h"
#include "impeller/playground/playground_test.h"
#include "impeller/renderer/pipeline_library.h"
#include "impeller/renderer/render_pass.h"

#include "gtest/gtest.h"
#include "third_party/imgui/imgui.h"

namespace impeller {
namespace testing {

static void InstantiateTestShaderLibrary(Context::BackendType backend_type) {
  auto fixture =
      flutter::testing::OpenFixtureAsMapping("playground.shaderbundle");
  auto library = flutter::gpu::ShaderLibrary::MakeFromFlatbuffer(
      backend_type, std::move(fixture));
  flutter::gpu::ShaderLibrary::SetOverride(library);
}

class RendererDartTest : public PlaygroundTest,
                         public flutter::testing::DartFixture {
 public:
  RendererDartTest()
      : settings_(CreateSettingsForFixture()),
        vm_ref_(flutter::DartVMRef::Create(settings_)) {
    fml::MessageLoop::EnsureInitializedForCurrentThread();

    current_task_runner_ = fml::MessageLoop::GetCurrent().GetTaskRunner();

    isolate_ = CreateDartIsolate();
    assert(isolate_);
    assert(isolate_->get()->GetPhase() == flutter::DartIsolate::Phase::Running);
  }

  flutter::testing::AutoIsolateShutdown* GetIsolate() {
    // Sneak the context into the Flutter GPU API.
    assert(GetContext() != nullptr);
    flutter::gpu::Context::SetOverrideContext(GetContext());

    InstantiateTestShaderLibrary(GetContext()->GetBackendType());

    return isolate_.get();
  }

  /// @brief  Run a Dart function that's expected to create a texture and pass
  ///         it back for rendering via `drawToPlayground`.
  std::shared_ptr<Texture> GetRenderedTextureFromDart(
      const char* dart_function_name) {
    std::shared_ptr<Texture> result;

    // Note: The Dart isolate is configured (by
    //       `RendererDartTest::CreateDartIsolate`) to use the main thread, so
    //       there's no need to synchronize with a latch here.
    auto draw_to_playground_surface = [&result](Dart_NativeArguments args) {
      flutter::gpu::Texture* texture =
          tonic::DartConverter<flutter::gpu::Texture*>::FromDart(
              Dart_GetNativeArgument(args, 0));
      assert(texture != nullptr);  // Should always be a valid pointer.
      result = texture->GetTexture();
    };
    AddNativeCallback("DrawToPlaygroundSurface",
                      CREATE_NATIVE_ENTRY(draw_to_playground_surface));

    bool success =
        GetIsolate()->RunInIsolateScope([&dart_function_name]() -> bool {
          if (tonic::CheckAndHandleError(::Dart_Invoke(
                  Dart_RootLibrary(), tonic::ToDart(dart_function_name), 0,
                  nullptr))) {
            return false;
          }
          return true;
        });
    if (!success) {
      FML_LOG(ERROR) << "Failed to invoke dart test function:"
                     << dart_function_name;
      return nullptr;
    }
    if (!result) {
      FML_LOG(ERROR) << "Dart test function `" << dart_function_name
                     << "` did not invoke `drawToPlaygroundSurface`.";
      return nullptr;
    }
    return result;
  }

  /// @brief  Call a dart function that produces a texture and render the result
  ///         in the playground.
  bool RenderDartToPlayground(const char* dart_function_name) {
    auto context = GetContext();
    assert(context != nullptr);

    //------------------------------------------------------------------------------
    /// Prepare pipeline.
    ///

    using TextureVS = TextureVertexShader;
    using TextureFS = TextureFragmentShader;
    using TexturePipelineBuilder = PipelineBuilder<TextureVS, TextureFS>;

    auto pipeline_desc =
        TexturePipelineBuilder::MakeDefaultPipelineDescriptor(*context);
    if (!pipeline_desc.has_value()) {
      FML_LOG(ERROR) << "Failed to create default pipeline descriptor.";
      return false;
    }
    pipeline_desc->SetSampleCount(SampleCount::kCount4);
    pipeline_desc->SetStencilAttachmentDescriptors(std::nullopt);

    auto pipeline =
        context->GetPipelineLibrary()->GetPipeline(pipeline_desc).Get();
    if (!pipeline || !pipeline->IsValid()) {
      FML_LOG(ERROR) << "Failed to create default pipeline.";
      return false;
    }

    //------------------------------------------------------------------------------
    /// Prepare vertex data.
    ///

    Size size = Size(GetWindowSize());

    VertexBufferBuilder<TextureVS::PerVertexData> texture_vtx_builder;

    // Always stretch out the texture to fit the screen.

    // clang-format off
    texture_vtx_builder.AddVertices({
        {{0.0,        0.0,         0.0}, {0.0, 0.0}},  // 1
        {{size.width, 0.0,         0.0}, {1.0, 0.0}},  // 2
        {{size.width, size.height, 0.0}, {1.0, 1.0}},  // 3
        {{0.0,        0.0,         0.0}, {0.0, 0.0}},  // 1
        {{size.width, size.height, 0.0}, {1.0, 1.0}},  // 3
        {{0.0,        size.height, 0.0}, {0.0, 1.0}},  // 4
    });
    // clang-format on

    //------------------------------------------------------------------------------
    /// Prepare sampler.
    ///

    const auto& sampler = context->GetSamplerLibrary()->GetSampler({});
    if (!sampler) {
      FML_LOG(ERROR) << "Failed to create default sampler.";
      return false;
    }

    //------------------------------------------------------------------------------
    /// Render to playground.
    ///

    SinglePassCallback callback = [&](RenderPass& pass) {
      auto texture = GetRenderedTextureFromDart(dart_function_name);
      if (!texture) {
        return false;
      }

      auto buffer = HostBuffer::Create(context->GetResourceAllocator());

      pass.SetPipeline(pipeline);
      pass.SetVertexBuffer(texture_vtx_builder.CreateVertexBuffer(
          *context->GetResourceAllocator()));

      TextureVS::UniformBuffer uniforms;
      uniforms.mvp = Matrix::MakeOrthographic(pass.GetRenderTargetSize()) *
                     Matrix::MakeScale(GetContentScale());
      TextureVS::BindUniformBuffer(pass, buffer->EmplaceUniform(uniforms));
      TextureFS::BindTextureContents(pass, texture, sampler);

      if (!pass.Draw().ok()) {
        return false;
      }
      return true;
    };
    return OpenPlaygroundHere(callback);
  }

 private:
  std::unique_ptr<flutter::testing::AutoIsolateShutdown> CreateDartIsolate() {
    const auto settings = CreateSettingsForFixture();
    flutter::TaskRunners task_runners(flutter::testing::GetCurrentTestName(),
                                      current_task_runner_,  //
                                      current_task_runner_,  //
                                      current_task_runner_,  //
                                      current_task_runner_   //
    );
    return flutter::testing::RunDartCodeInIsolate(
        vm_ref_, settings, task_runners, "main", {},
        flutter::testing::GetDefaultKernelFilePath());
  }

  const flutter::Settings settings_;
  flutter::DartVMRef vm_ref_;
  fml::RefPtr<fml::TaskRunner> current_task_runner_;
  std::unique_ptr<flutter::testing::AutoIsolateShutdown> isolate_;
};

INSTANTIATE_PLAYGROUND_SUITE(RendererDartTest);

TEST_P(RendererDartTest, canCreateRenderPassAndSubmit) {
  ASSERT_TRUE(RenderDartToPlayground("canCreateRenderPassAndSubmit"));
}

TEST_P(RendererDartTest, CanRunDartInPlaygroundFrame) {
  auto isolate = GetIsolate();

  SinglePassCallback callback = [&](RenderPass& pass) {
    ImGui::Begin("Dart test", nullptr);
    ImGui::Text(
        "This test executes Dart code during the playground frame callback.");
    ImGui::End();

    return isolate->RunInIsolateScope([]() -> bool {
      if (tonic::CheckAndHandleError(::Dart_Invoke(
              Dart_RootLibrary(), tonic::ToDart("sayHi"), 0, nullptr))) {
        return false;
      }
      return true;
    });
  };
  ASSERT_TRUE(OpenPlaygroundHere(callback));
}

TEST_P(RendererDartTest, CanInstantiateFlutterGPUContext) {
  auto isolate = GetIsolate();
  bool result = isolate->RunInIsolateScope([]() -> bool {
    if (tonic::CheckAndHandleError(::Dart_Invoke(
            Dart_RootLibrary(), tonic::ToDart("instantiateDefaultContext"), 0,
            nullptr))) {
      return false;
    }
    return true;
  });

  ASSERT_TRUE(result);
}

#define DART_TEST_CASE(name)                                            \
  TEST_P(RendererDartTest, name) {                                      \
    auto isolate = GetIsolate();                                        \
    bool result = isolate->RunInIsolateScope([]() -> bool {             \
      if (tonic::CheckAndHandleError(::Dart_Invoke(                     \
              Dart_RootLibrary(), tonic::ToDart(#name), 0, nullptr))) { \
        return false;                                                   \
      }                                                                 \
      return true;                                                      \
    });                                                                 \
    ASSERT_TRUE(result);                                                \
  }

/// These test entries correspond to Dart functions located in
/// `flutter/impeller/fixtures/dart_tests.dart`

DART_TEST_CASE(canCreateShaderLibrary);
DART_TEST_CASE(canReflectUniformStructs);
DART_TEST_CASE(uniformBindFailsForInvalidHostBufferOffset);

// DART_TEST_CASE(canCreateRenderPassAndSubmit);

}  // namespace testing
}  // namespace impeller
