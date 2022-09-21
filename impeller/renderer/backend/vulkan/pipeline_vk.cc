// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/pipeline_vk.h"
#include "vulkan/vulkan_handles.hpp"

namespace impeller {

PipelineCreateInfoVK::PipelineCreateInfoVK(
    vk::UniquePipeline pipeline,
    vk::UniqueRenderPass render_pass,
    vk::UniquePipelineLayout layout,
    vk::UniqueDescriptorSetLayout descriptor_set_layout)
    : pipeline_(std::move(pipeline)),
      render_pass_(std::move(render_pass)),
      pipeline_layout_(std::move(layout)),
      descriptor_set_layout_(std::move(descriptor_set_layout)) {
  is_valid_ =
      pipeline_ && render_pass_ && pipeline_layout_ && descriptor_set_layout_;
}

bool PipelineCreateInfoVK::IsValid() const {
  return is_valid_;
}

vk::Pipeline PipelineCreateInfoVK::GetPipeline() {
  return *pipeline_;
}

vk::RenderPass PipelineCreateInfoVK::GetRenderPass() {
  return *render_pass_;
}

vk::PipelineLayout PipelineCreateInfoVK::GetPipelineLayout() {
  return *pipeline_layout_;
}

vk::DescriptorSetLayout PipelineCreateInfoVK::GetDescriptorSetLayout() {
  return *descriptor_set_layout_;
}

PipelineVK::PipelineVK(std::weak_ptr<PipelineLibrary> library,
                       PipelineDescriptor desc,
                       std::unique_ptr<PipelineCreateInfoVK> create_info)
    : Pipeline(std::move(library), std::move(desc)),
      pipeline_info_(std::move(create_info)) {
  FML_LOG(ERROR) << "created pipeline " << desc.GetLabel()
                 << ", addr: " << pipeline_info_->GetPipeline();
}

PipelineVK::~PipelineVK() {
  FML_LOG(ERROR) << "destroyed pipeline " << GetDescriptor().GetLabel()
                 << ", addr: " << pipeline_info_->GetPipeline();
}

bool PipelineVK::IsValid() const {
  return pipeline_info_->IsValid();
}

PipelineCreateInfoVK* PipelineVK::GetCreateInfo() {
  return pipeline_info_.get();
}

}  // namespace impeller
