// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/command_queue.h"

namespace impeller {

fml::Status CommandQueue::Submit(
    const std::vector<std::shared_ptr<CommandBuffer>>& buffers,
    const CompletionCallback& cb) {
  if (buffers.empty()) {
    return fml::Status(fml::StatusCode::kInvalidArgument,
                       "No command buffers provided.");
  }
  for (const std::shared_ptr<CommandBuffer>& buffer : buffers) {
    if (!buffer->SubmitCommands(cb)) {
      return fml::Status(fml::StatusCode::kCancelled,
                         "Failed to submit command buffer.");
    }
  }
  return fml::Status();
}

}  // namespace impeller
