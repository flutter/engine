// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_CORE_GRAPHICS_QUEUE_H_
#define FLUTTER_IMPELLER_CORE_GRAPHICS_QUEUE_H_

#include <functional>

#include "fml/status.h"
#include "impeller/renderer/command_buffer.h"

namespace impeller {

/// @brief An interface for submitting command buffers to the GPU for
///        encoding and execution.
class GraphicsQueue {
 public:
  using CompletionCallback = std::function<void(CommandBuffer::Status)>;

  GraphicsQueue() = default;

  virtual ~GraphicsQueue() = default;

  /// @brief Submit one or more command buffer objects to be encoded and
  ///        executed on the GPU.
  ///
  ///        The order of the provided buffers determines the ordering in which
  ///        they are submitted.
  ///
  ///        Optionally accepts a ccallback that will fire with an updated
  ///        status based on encoding state. Only the Metal and Vulkan backend
  ///        can give a status beyond successful encoding. This callback may
  ///        be called more than once.
  virtual fml::Status Submit(
      const std::vector<std::shared_ptr<CommandBuffer>>& buffers,
      const CompletionCallback& cb = {});

 private:
  GraphicsQueue(const GraphicsQueue&) = delete;

  GraphicsQueue& operator=(const GraphicsQueue&) = delete;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_CORE_GRAPHICS_QUEUE_H_
