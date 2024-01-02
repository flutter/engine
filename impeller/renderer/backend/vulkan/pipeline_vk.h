// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_PIPELINE_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_PIPELINE_VK_H_

#include <future>
#include <memory>

#include "impeller/base/backend_cast.h"
#include "impeller/base/thread.h"
#include "impeller/renderer/backend/vulkan/device_holder.h"
#include "impeller/renderer/backend/vulkan/formats_vk.h"
#include "impeller/renderer/backend/vulkan/pipeline_cache_vk.h"
#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/pipeline.h"

namespace impeller {

class PipelineVK final
    : public Pipeline<PipelineDescriptor>,
      public BackendCast<PipelineVK, Pipeline<PipelineDescriptor>> {
 public:
  static std::unique_ptr<PipelineVK> Create(
      const PipelineDescriptor& desc,
      const std::shared_ptr<DeviceHolder>& device_holder,
      std::weak_ptr<PipelineLibrary> weak_library,
      SubpassCursorVK subpass_cursor);

  // |Pipeline|
  ~PipelineVK() override;

  void PreloadPipeline(SubpassCursorVK cursor) const;

  bool HasPreloadedPipeline(SubpassCursorVK cursor) const;

  vk::Pipeline GetPipeline(SubpassCursorVK cursor) const;

  const vk::PipelineLayout& GetPipelineLayout() const;

  const vk::DescriptorSetLayout& GetDescriptorSetLayout() const;

 private:
  friend class PipelineLibraryVK;

  using SubpassPipelines =
      std::unordered_map<SubpassCursorVK,
                         std::shared_future<std::shared_ptr<PipelineVK>>,
                         SubpassCursorVK::Hash,
                         SubpassCursorVK::Equal>;

  std::weak_ptr<DeviceHolder> device_holder_;
  vk::UniquePipeline pipeline_;
  vk::UniqueRenderPass render_pass_;
  vk::UniquePipelineLayout layout_;
  vk::UniqueDescriptorSetLayout descriptor_set_layout_;
  SubpassCursorVK subpass_cursor_;
  mutable Mutex subpass_pipelines_mutex_;
  mutable SubpassPipelines subpass_pipelines_ IPLR_GUARDED_BY(
      subpass_pipelines_mutex_);

  bool is_valid_ = false;

  PipelineVK(std::weak_ptr<DeviceHolder> device_holder,
             std::weak_ptr<PipelineLibrary> library,
             const PipelineDescriptor& desc,
             vk::UniquePipeline pipeline,
             vk::UniqueRenderPass render_pass,
             vk::UniquePipelineLayout layout,
             vk::UniqueDescriptorSetLayout descriptor_set_layout,
             SubpassCursorVK subpass_cursor);

  // |Pipeline|
  bool IsValid() const override;

  PipelineVK(const PipelineVK&) = delete;

  PipelineVK& operator=(const PipelineVK&) = delete;

  std::shared_future<std::shared_ptr<PipelineVK>> CreateOrGetVariantForSubpass(
      SubpassCursorVK cursor) const;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_PIPELINE_VK_H_
