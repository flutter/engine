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
  CommandBufferVK(VULKAN_HPP_DEFAULT_DISPATCHER_TYPE& dispatch,
                  vk::CommandBuffer buffer);

  // |CommandBuffer|
  ~CommandBufferVK() override;

 private:
  friend class ContextVK;

  const VULKAN_HPP_DEFAULT_DISPATCHER_TYPE& dispatch_;
  vk::CommandBuffer buffer_;

  // |CommandBuffer|
  void SetLabel(const std::string& label) const override;

  // |CommandBuffer|
  bool IsValid() const override;

  // |CommandBuffer|
  bool SubmitCommands(CompletionCallback callback) override;

  // |CommandBuffer|
  std::shared_ptr<RenderPass> OnCreateRenderPass(
      RenderTarget target) const override;

  FML_DISALLOW_COPY_AND_ASSIGN(CommandBufferVK);
};

}  // namespace impeller
