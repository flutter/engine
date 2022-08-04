// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/command_buffer.h"

namespace impeller {

class CommandBufferVK final : public CommandBuffer {
 public:
  static std::shared_ptr<CommandBufferVK> Create(vk::Device device,
                                                 vk::CommandPool command_pool);

  explicit CommandBufferVK(vk::Device device,
                           vk::UniqueCommandBuffer command_buffer);

  // |CommandBuffer|
  ~CommandBufferVK() override;

 private:
  friend class ContextMTL;

  vk::Device device_;
  vk::UniqueCommandBuffer command_buffer_;

  // |CommandBuffer|
  void SetLabel(const std::string& label) const override;

  // |CommandBuffer|
  bool IsValid() const override;

  // |CommandBuffer|
  bool SubmitCommands(CompletionCallback callback) override;

  // |CommandBuffer|
  std::shared_ptr<RenderPass> OnCreateRenderPass(
      RenderTarget target) const override;

  // |CommandBuffer|
  std::shared_ptr<BlitPass> OnCreateBlitPass() const override;

  FML_DISALLOW_COPY_AND_ASSIGN(CommandBufferVK);
};

}  // namespace impeller
