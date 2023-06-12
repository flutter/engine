// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "impeller/renderer/backend/vulkan/command_buffer_cache.h"

namespace impeller {
namespace testing {

namespace {
struct Tallies {
  int32_t bindPipeline_count = 0;
  int32_t setStencilReference_count = 0;
  int32_t setScissor_count = 0;
  int32_t setViewport_count = 0;
};

class MockCommandBuffer {
 public:
  MockCommandBuffer() : tallies_(new Tallies()) {}

  void bindPipeline(vk::PipelineBindPoint pipeline_bind_point,
                    vk::Pipeline pipeline) {
    tallies_->bindPipeline_count += 1;
  }

  void setStencilReference(vk::StencilFaceFlags face_mask, uint32_t reference) {
    tallies_->setStencilReference_count += 1;
  }

  void setScissor(uint32_t first_scissor,
                  uint32_t scissor_count,
                  const vk::Rect2D* scissors) {
    tallies_->setScissor_count += 1;
  }

  void setViewport(uint32_t first_viewport,
                   uint32_t viewport_count,
                   const vk::Viewport* viewports) {
    tallies_->setViewport_count += 1;
  }

  std::shared_ptr<Tallies> tallies_;
};
}  // namespace

TEST(CommandBufferCacheTest, bindPipeline) {
  CommandBufferCache<MockCommandBuffer> cache;
  MockCommandBuffer buffer;
  VkPipeline vk_pipeline = reinterpret_cast<VkPipeline>(0xfeedface);
  vk::Pipeline pipeline(vk_pipeline);
  ASSERT_EQ(buffer.tallies_->bindPipeline_count, 0);
  cache.bindPipeline(buffer, vk::PipelineBindPoint::eGraphics, pipeline);
  ASSERT_EQ(buffer.tallies_->bindPipeline_count, 1);
  cache.bindPipeline(buffer, vk::PipelineBindPoint::eGraphics, pipeline);
  ASSERT_EQ(buffer.tallies_->bindPipeline_count, 1);
}

TEST(CommandBufferCacheTest, setStencilReference) {
  CommandBufferCache<MockCommandBuffer> cache;
  MockCommandBuffer buffer;
  ASSERT_EQ(buffer.tallies_->setStencilReference_count, 0);
  cache.setStencilReference(
      buffer, vk::StencilFaceFlagBits::eVkStencilFrontAndBack, 123);
  ASSERT_EQ(buffer.tallies_->setStencilReference_count, 1);
  cache.setStencilReference(
      buffer, vk::StencilFaceFlagBits::eVkStencilFrontAndBack, 123);
  ASSERT_EQ(buffer.tallies_->setStencilReference_count, 1);
}

TEST(CommandBufferCacheTest, setScissor) {
  CommandBufferCache<MockCommandBuffer> cache;
  MockCommandBuffer buffer;
  vk::Rect2D scissors;
  ASSERT_EQ(buffer.tallies_->setScissor_count, 0);
  cache.setScissor(buffer, 0, 1, &scissors);
  ASSERT_EQ(buffer.tallies_->setScissor_count, 1);
  cache.setScissor(buffer, 0, 1, &scissors);
  ASSERT_EQ(buffer.tallies_->setScissor_count, 1);
}

TEST(CommandBufferCacheTest, setViewport) {
  CommandBufferCache<MockCommandBuffer> cache;
  MockCommandBuffer buffer;
  vk::Viewport viewports;
  ASSERT_EQ(buffer.tallies_->setViewport_count, 0);
  cache.setViewport(buffer, 0, 1, &viewports);
  ASSERT_EQ(buffer.tallies_->setViewport_count, 1);
  cache.setViewport(buffer, 0, 1, &viewports);
  ASSERT_EQ(buffer.tallies_->setViewport_count, 1);
}

}  // namespace testing
}  // namespace impeller
