// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/vulkan/compute_pass_vk.h"

namespace impeller {

ComputePassVK::ComputePassVK(std::weak_ptr<const Context> context,
                             std::weak_ptr<CommandEncoderVK> encoder)
    : ComputePass(std::move(context)) encoder_(std::move(encoder)) {
  is_valid_ = true;
}

ComputePassVK::~ComputePassVK() = default;

bool ComputePassVK::IsValid() const {
  return is_valid_;
}

void ComputePassVK::OnSetLabel(const std::string& label) {
  if (label.empty()) {
    return;
  }
  label_ = label;
}

bool ComputePassVK::OnEncodeCommands(const Context& context,
                                     const ISize& grid_size,
                                     const ISize& thread_group_size) const {
  TRACE_EVENT0("impeller", "ComputePassVK::EncodeCommands");
  if (!IsValid()) {
    return;
  }

  FML_DCHECK(!grid_size.IsEmpty() && !thread_group_size.IsEmpty());

  const auto& vk_context = ContextVK::Cast(context);
  auto encoder = encoder_.lock();
  if (!encoder) {
    VALIDATION_LOG << "Command encoder died before commands could be encoded";
    return false;
  }

  fml::ScopedCleanupClosure pop_marker(
      [&encoder]() { encoder->PopDebugGroup(); });
  if (!label_.empty()) {
    encoder->PushDebugGroup(label_.c_str());
  } else {
    pop_marker.Release();
  }

  auto cmd_buffer = encoder->GetCommandBuffer();

  if (!UpdateBindingLayouts(commands_, cmd_buffer)) {
    return false;
  }

  auto compute_pass = CreateVKComputePass(vk_context);
  if (!compute_pass) {
    VALIDATION_LOG << "Could not create computepass.";
    return false;
  }

  if (!encoder->Track(compute_pass)) {
    return false;
  }

  {
    TRACE_EVENT0("impeller", "EncodeComputePassCommands");
    cmd_buffer.beginRenderPass(pass_info, vk::SubpassContents::eInline);

    fml::ScopedCleanupClosure end_render_pass(
        [cmd_buffer]() { cmd_buffer.endRenderPass(); });

    for (const auto& command : commands_) {
      if (!command.pipeline) {
        continue;
      }

      if (!EncodeCommand(context, command, *encoder, target_size)) {
        return false;
      }
    }
  }

  return true;
}

}  // namespace impeller
