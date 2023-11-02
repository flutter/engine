// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "gmock/gmock.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/filters/directional_gaussian_blur_filter_contents.h"
#include "impeller/renderer/testing/mocks.h"

namespace impeller {
namespace testing {

using ::testing::_;
using ::testing::An;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::ReturnRef;

namespace {

Scalar CalculateSigmaForBlurRadius(Scalar blur_radius) {
  // See Sigma.h
  return (blur_radius / kKernelRadiusPerSigma) + 0.5;
}
}  // namespace

class DirectionalGaussianBlurFilterContentsTest : public ::testing::Test {
 public:
  void SetUp() override { capabilities_ = mock_capabilities_; }

  std::shared_ptr<MockCapabilities> mock_capabilities_ =
      std::make_shared<MockCapabilities>();
  std::shared_ptr<const Capabilities> capabilities_;
};

TEST_F(DirectionalGaussianBlurFilterContentsTest, CoverageWithEffectTransform) {
  TextureDescriptor desc = {
      .size = ISize(100, 100),
  };
  Scalar sigma_radius_1 = CalculateSigmaForBlurRadius(1.0);
  auto contents = std::make_unique<DirectionalGaussianBlurFilterContents>();
  contents->SetSigma(Sigma{sigma_radius_1});
  contents->SetDirection({1.0, 0.0});
  std::shared_ptr<MockTexture> texture = std::make_shared<MockTexture>(desc);
  EXPECT_CALL(*texture, GetSize()).WillRepeatedly(Return(ISize(100, 100)));
  FilterInput::Vector inputs = {FilterInput::Make(texture)};
  Entity entity;
  entity.SetTransformation(Matrix::MakeTranslation({100, 100, 0}));
  std::optional<Rect> coverage = contents->GetFilterCoverage(
      inputs, entity, /*effect_transform=*/Matrix::MakeScale({2.0, 2.0, 1.0}));
  ASSERT_EQ(coverage, Rect::MakeLTRB(100 - 2, 100, 200 + 2, 200));
}

TEST_F(DirectionalGaussianBlurFilterContentsTest, FilterSourceCoverage) {
  Scalar sigma_radius_1 = CalculateSigmaForBlurRadius(1.0);
  auto contents = std::make_unique<DirectionalGaussianBlurFilterContents>();
  contents->SetSigma(Sigma{sigma_radius_1});
  contents->SetDirection({1.0, 0.0});
  std::optional<Rect> coverage = contents->GetFilterSourceCoverage(
      /*effect_transform=*/Matrix::MakeScale({2.0, 2.0, 1.0}),
      /*output_limit=*/Rect::MakeLTRB(100, 100, 200, 200));
  ASSERT_EQ(coverage, Rect::MakeLTRB(100 - 2, 100, 200 + 2, 200));
}

TEST_F(DirectionalGaussianBlurFilterContentsTest, RenderNoCoverage) {
  Scalar sigma_radius_1 = CalculateSigmaForBlurRadius(1.0);
  auto contents = std::make_unique<DirectionalGaussianBlurFilterContents>();
  contents->SetSigma(Sigma{sigma_radius_1});
  contents->SetDirection({1.0, 0.0});
  auto mock_context = std::make_shared<MockImpellerContext>();
  auto mock_typographer_context = std::make_shared<MockTypographerContext>();
  auto mock_allocator = std::make_shared<MockAllocator>();
  auto mock_render_target_allocator =
      std::make_shared<MockRenderTargetAllocator>(mock_allocator);
  ContentContext renderer(mock_context, mock_typographer_context,
                          mock_render_target_allocator);
  Entity entity;
  Rect coverage_hint = Rect::MakeLTRB(0, 0, 0, 0);
  std::optional<Entity> result =
      contents->GetEntity(renderer, entity, coverage_hint);
  ASSERT_FALSE(result.has_value());
}

TEST_F(DirectionalGaussianBlurFilterContentsTest, RenderSomething) {
  TextureDescriptor desc = {
      .size = ISize(100, 100),
  };
  std::shared_ptr<MockTexture> texture = std::make_shared<MockTexture>(desc);
  EXPECT_CALL(*texture, GetSize()).WillRepeatedly(Return(ISize(100, 100)));
  Scalar sigma_radius_1 = CalculateSigmaForBlurRadius(1.0);
  auto contents = std::make_unique<DirectionalGaussianBlurFilterContents>();
  contents->SetSigma(Sigma{sigma_radius_1});
  contents->SetDirection({1.0, 0.0});
  contents->SetInputs({FilterInput::Make(texture)});
  auto mock_context = std::make_shared<MockImpellerContext>();
  EXPECT_CALL(*mock_context, GetCapabilities())
      .WillRepeatedly(ReturnRef(capabilities_));
  EXPECT_CALL(*mock_context, IsValid()).WillRepeatedly(Return(true));
  auto mock_sampler_library = std::make_shared<MockSamplerLibrary>();
  auto mock_shader_library = std::make_shared<MockShaderLibrary>();
  auto mock_pipeline_library = std::make_shared<MockPipelineLibrary>();
  EXPECT_CALL(*mock_pipeline_library, GetPipeline(An<PipelineDescriptor>()))
      .WillRepeatedly(
          Invoke([&mock_pipeline_library](PipelineDescriptor descriptor) {
            PipelineFuture<PipelineDescriptor> result;
            std::promise<std::shared_ptr<Pipeline<PipelineDescriptor>>> promise;
            auto mock_pipeline =
                std::make_shared<MockPipeline<PipelineDescriptor>>(
                    mock_pipeline_library, descriptor);
            EXPECT_CALL(*mock_pipeline, IsValid()).WillRepeatedly(Return(true));
            promise.set_value(mock_pipeline);
            result.descriptor = descriptor;
            result.future = promise.get_future();
            return result;
          }));
  EXPECT_CALL(*mock_shader_library, GetFunction(_, _))
      .WillRepeatedly(Invoke([](std::string_view name, ShaderStage stage) {
        return std::make_shared<MockShaderFunction>(UniqueID(),
                                                    std::string(name), stage);
      }));
  EXPECT_CALL(*mock_context, GetSamplerLibrary())
      .WillRepeatedly(Return(mock_sampler_library));
  EXPECT_CALL(*mock_context, GetShaderLibrary())
      .WillRepeatedly(Return(mock_shader_library));
  EXPECT_CALL(*mock_context, GetPipelineLibrary())
      .WillRepeatedly(Return(mock_pipeline_library));
  EXPECT_CALL(*mock_context, CreateCommandBuffer())
      .WillRepeatedly(Invoke(([&mock_context]() {
        auto result = std::make_shared<MockCommandBuffer>(mock_context);
        EXPECT_CALL(*result, IsValid()).WillRepeatedly(Return(true));
        EXPECT_CALL(*result, OnSubmitCommands(_)).WillRepeatedly(Return(true));
        EXPECT_CALL(*result, OnCreateRenderPass(_))
            .WillRepeatedly(
                Invoke(([&mock_context](const RenderTarget& render_target) {
                  auto result = std::make_shared<MockRenderPass>(mock_context,
                                                                 render_target);
                  EXPECT_CALL(*result, IsValid).WillRepeatedly(Return(true));
                  EXPECT_CALL(*result, OnEncodeCommands(_))
                      .WillRepeatedly(Return(true));
                  return result;
                })));
        return result;
      })));
  auto mock_typographer_context = std::make_shared<MockTypographerContext>();
  auto mock_allocator = std::make_shared<MockAllocator>();
  auto mock_render_target_allocator =
      std::make_shared<MockRenderTargetAllocator>(mock_allocator);
  EXPECT_CALL(*mock_render_target_allocator, CreateTexture(_))
      .WillRepeatedly(Invoke(([](const TextureDescriptor& desc) {
        auto result = std::make_shared<MockTexture>(desc);
        EXPECT_CALL(*result, IsValid()).WillRepeatedly(Return(true));
        return result;
      })));
  ContentContext renderer(mock_context, mock_typographer_context,
                          mock_render_target_allocator);
  Entity entity;
  Rect coverage_hint = Rect::MakeLTRB(0, 0, 0, 0);
  std::optional<Entity> result =
      contents->GetEntity(renderer, entity, coverage_hint);
  ASSERT_TRUE(result.has_value());
  ASSERT_EQ(result.value().GetBlendMode(), BlendMode::kSourceOver);
}

}  // namespace testing
}  // namespace impeller
