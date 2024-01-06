// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/compute_pass.h"
#include <memory>

#include "impeller/base/strings.h"
#include "impeller/base/validation.h"
#include "impeller/core/host_buffer.h"

namespace impeller {

ComputePass::ComputePass(std::weak_ptr<const Context> context)
    : context_(std::move(context)), transients_buffer_(HostBuffer::Create()) {}

ComputePass::~ComputePass() = default;

HostBuffer& ComputePass::GetTransientsBuffer() {
  return *transients_buffer_;
}

void ComputePass::SetLabel(const std::string& label) {
  if (label.empty()) {
    return;
  }
  transients_buffer_->SetLabel(SPrintF("%s Transients", label.c_str()));
  OnSetLabel(label);
}

void ComputePass::SetGridSize(const ISize& size) {
  grid_size_ = size;
}

void ComputePass::SetThreadGroupSize(const ISize& size) {
  thread_group_size_ = size;
}

bool ComputePass::AddCommand(ComputeCommand command) {
  if (!command) {
    VALIDATION_LOG
        << "Attempted to add an invalid command to the compute pass.";
    return false;
  }

  auto buffer_offset = bound_buffers_.size();
  auto buffer_length = command.bindings.buffer_offset;
  auto texture_offset = bound_textures_.size();
  auto texture_length = command.bindings.texture_offset;

  for (auto i = 0u; i < buffer_length; i++) {
    bound_buffers_.push_back(std::move(command.bindings.bound_buffers[i]));
  }
  for (auto i = 0u; i < texture_length; i++) {
    bound_textures_.push_back(std::move(command.bindings.bound_textures[i]));
  }

  commands_.emplace_back(BoundComputeCommand{
      .pipeline = std::move(command.pipeline),
      .buffer_offset = buffer_offset,
      .buffer_length = buffer_length,
      .texture_offset = texture_offset,
      .texture_length = texture_length,
#ifdef IMPELLER_DEBUG
      .label = std::move(command.label)
#endif  // IMPELLER_DEBUG
  });
  return true;
}

bool ComputePass::EncodeCommands() const {
  if (grid_size_.IsEmpty() || thread_group_size_.IsEmpty()) {
    FML_DLOG(WARNING) << "Attempted to encode a compute pass with an empty "
                         "grid or thread group size.";
    return false;
  }
  auto context = context_.lock();
  // The context could have been collected in the meantime.
  if (!context) {
    return false;
  }
  return OnEncodeCommands(*context, grid_size_, thread_group_size_);
}

}  // namespace impeller
