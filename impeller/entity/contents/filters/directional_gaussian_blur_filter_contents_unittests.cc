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
  void SetUp() override {
    mock_render_target_allocator_ =
        std::make_shared<MockRenderTargetAllocator>(mock_allocator_);
    capabilities_ = mock_capabilities_;
  }

  // Stubs in the minimal support to make rendering pass.
  void SetupMinimalContext() {
    EXPECT_CALL(*mock_context_, GetCapabilities())
        .WillRepeatedly(ReturnRef(capabilities_));
    EXPECT_CALL(*mock_context_, IsValid()).WillRepeatedly(Return(true));
    EXPECT_CALL(*mock_pipeline_library_, GetPipeline(An<PipelineDescriptor>()))
        .WillRepeatedly(Invoke([mock_pipeline_library =
                                    std::weak_ptr<MockPipelineLibrary>(
                                        mock_pipeline_library_)](
                                   PipelineDescriptor descriptor) {
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
    EXPECT_CALL(*mock_shader_library_, GetFunction(_, _))
        .WillRepeatedly(Invoke([](std::string_view name, ShaderStage stage) {
          return std::make_shared<MockShaderFunction>(UniqueID(),
                                                      std::string(name), stage);
        }));
    EXPECT_CALL(*mock_context_, GetSamplerLibrary())
        .WillRepeatedly(Return(mock_sampler_library_));
    EXPECT_CALL(*mock_context_, GetShaderLibrary())
        .WillRepeatedly(Return(mock_shader_library_));
    EXPECT_CALL(*mock_context_, GetPipelineLibrary())
        .WillRepeatedly(Return(mock_pipeline_library_));
    EXPECT_CALL(*mock_context_, CreateCommandBuffer())
        .WillRepeatedly(Invoke(([mock_context =
                                     std::weak_ptr<MockImpellerContext>(
                                         mock_context_)]() {
          auto result = std::make_shared<MockCommandBuffer>(mock_context);
          EXPECT_CALL(*result, IsValid()).WillRepeatedly(Return(true));
          EXPECT_CALL(*result, OnSubmitCommands(_))
              .WillRepeatedly(Return(true));
          EXPECT_CALL(*result, OnCreateRenderPass(_))
              .WillRepeatedly(
                  Invoke(([mock_context](const RenderTarget& render_target) {
                    auto result = std::make_shared<MockRenderPass>(
                        mock_context, render_target);
                    EXPECT_CALL(*result, IsValid).WillRepeatedly(Return(true));
                    EXPECT_CALL(*result, OnEncodeCommands(_))
                        .WillRepeatedly(Return(true));
                    return result;
                  })));
          return result;
        })));
    EXPECT_CALL(*mock_render_target_allocator_, CreateTexture(_))
        .WillRepeatedly(Invoke(([](const TextureDescriptor& desc) {
          auto result = std::make_shared<MockTexture>(desc);
          EXPECT_CALL(*result, IsValid()).WillRepeatedly(Return(true));
          EXPECT_CALL(*result, GetSize()).WillRepeatedly(Return(desc.size));
          return result;
        })));
  }

  std::shared_ptr<MockImpellerContext> mock_context_ =
      std::make_shared<MockImpellerContext>();
  std::shared_ptr<MockCapabilities> mock_capabilities_ =
      std::make_shared<MockCapabilities>();
  std::shared_ptr<MockSamplerLibrary> mock_sampler_library_ =
      std::make_shared<MockSamplerLibrary>();
  std::shared_ptr<MockShaderLibrary> mock_shader_library_ =
      std::make_shared<MockShaderLibrary>();
  std::shared_ptr<MockPipelineLibrary> mock_pipeline_library_ =
      std::make_shared<MockPipelineLibrary>();
  std::shared_ptr<MockTypographerContext> mock_typographer_context_ =
      std::make_shared<MockTypographerContext>();
  std::shared_ptr<MockAllocator> mock_allocator_ =
      std::make_shared<MockAllocator>();
  std::shared_ptr<MockRenderTargetAllocator> mock_render_target_allocator_;
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
  SetupMinimalContext();
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
  ContentContext renderer(mock_context_, mock_typographer_context_,
                          mock_render_target_allocator_);
  Entity entity;
  std::optional<Entity> result =
      contents->GetEntity(renderer, entity, /*coverage_hint=*/{});
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(result.value().GetBlendMode(), BlendMode::kSourceOver);
  std::optional<Rect> result_coverage = result.value().GetCoverage();
  std::optional<Rect> contents_coverage = contents->GetCoverage(entity);
  EXPECT_TRUE(result_coverage.has_value());
  EXPECT_TRUE(contents_coverage.has_value());
  EXPECT_NEAR(result_coverage.value().GetLeft(),
              contents_coverage.value().GetLeft(), kEhCloseEnough);
  EXPECT_NEAR(result_coverage.value().GetTop(),
              contents_coverage.value().GetTop(), kEhCloseEnough);
  EXPECT_NEAR(result_coverage.value().GetRight(),
              contents_coverage.value().GetRight(), kEhCloseEnough);
  EXPECT_NEAR(result_coverage.value().GetBottom(),
              contents_coverage.value().GetBottom(), kEhCloseEnough);
}

}  // namespace testing
}  // namespace impeller
