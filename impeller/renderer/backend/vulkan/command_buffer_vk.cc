// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/command_buffer_vk.h"

#include <memory>
#include <utility>

#include "flutter/fml/logging.h"
#include "flutter/fml/trace_event.h"
#include "impeller/base/validation.h"
#include "impeller/renderer/backend/vulkan/blit_pass_vk.h"
#include "impeller/renderer/backend/vulkan/command_encoder_vk.h"
#include "impeller/renderer/backend/vulkan/compute_pass_vk.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/backend/vulkan/formats_vk.h"
#include "impeller/renderer/backend/vulkan/render_pass_vk.h"
#include "impeller/renderer/command_buffer.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

CommandBufferVK::CommandBufferVK(std::weak_ptr<const Context> context,
                                 std::shared_ptr<CommandEncoderVK> encoder)
    : CommandBuffer(std::move(context)), encoder_(std::move(encoder)) {
  if (!encoder_ || !encoder_->IsValid()) {
    return;
  }
  is_valid_ = true;
}

CommandBufferVK::~CommandBufferVK() = default;

void CommandBufferVK::SetLabel(const std::string& label) const {
  auto context = context_.lock();
  if (!context || !encoder_) {
    return;
  }
  ContextVK::Cast(*context).SetDebugName(encoder_->GetCommandBuffer(), label);
}

bool CommandBufferVK::IsValid() const {
  return is_valid_;
}

const std::shared_ptr<CommandEncoderVK>& CommandBufferVK::GetEncoder() const {
  return encoder_;
}

bool CommandBufferVK::SubmitCommandsAsync(std::shared_ptr<BlitPass> blit_pass) {
  TRACE_EVENT0("impeller", "CommandBufferVK::SubmitCommandsAsync");
  if (!blit_pass->IsValid() || !IsValid()) {
    return false;
  }
  auto context = context_.lock();
  if (!context) {
    return false;
  }
  if (!blit_pass->EncodeCommands(context->GetResourceAllocator())) {
    return false;
  }

  const auto& context_vk = ContextVK::Cast(*context);
  if (!encoder_->Finish()) {
    return false;
  }
  context_vk.GetCommandBufferQueue()->Enqueue(std::move(encoder_));
  return true;
}

bool CommandBufferVK::SubmitCommandsAsync(
    std::shared_ptr<RenderPass> render_pass) {
  TRACE_EVENT0("impeller", "CommandBufferVK::SubmitCommandsAsync");
  if (!render_pass->IsValid() || !IsValid()) {
    return false;
  }
  const auto context = context_.lock();
  if (!context) {
    return false;
  }
  if (!render_pass->EncodeCommands()) {
    return false;
  }
  const auto& context_vk = ContextVK::Cast(*context);
  if (!encoder_->Finish()) {
    return false;
  }
  context_vk.GetCommandBufferQueue()->Enqueue(std::move(encoder_));
  return true;
}

bool CommandBufferVK::OnSubmitCommands(CompletionCallback callback) {
  if (!callback) {
    return encoder_->Submit();
  }
  return encoder_->Submit([callback](bool submitted) {
    callback(submitted ? CommandBuffer::Status::kCompleted
                       : CommandBuffer::Status::kError);
  });
}

void CommandBufferVK::OnWaitUntilScheduled() {}

std::shared_ptr<RenderPass> CommandBufferVK::OnCreateRenderPass(
    RenderTarget target) {
  auto context = context_.lock();
  if (!context) {
    return nullptr;
  }
  auto pass = std::shared_ptr<RenderPassVK>(new RenderPassVK(context,  //
                                                             target,   //
                                                             encoder_  //
                                                             ));
  if (!pass->IsValid()) {
    return nullptr;
  }
  return pass;
}

std::shared_ptr<BlitPass> CommandBufferVK::OnCreateBlitPass() const {
  if (!IsValid()) {
    return nullptr;
  }
  auto pass = std::shared_ptr<BlitPassVK>(new BlitPassVK(encoder_));
  if (!pass->IsValid()) {
    return nullptr;
  }
  return pass;
}

std::shared_ptr<ComputePass> CommandBufferVK::OnCreateComputePass() const {
  if (!IsValid()) {
    return nullptr;
  }
  auto context = context_.lock();
  if (!context) {
    return nullptr;
  }
  auto pass = std::shared_ptr<ComputePassVK>(new ComputePassVK(context,  //
                                                               encoder_  //
                                                               ));
  if (!pass->IsValid()) {
    return nullptr;
  }
  return pass;
}

}  // namespace impeller
