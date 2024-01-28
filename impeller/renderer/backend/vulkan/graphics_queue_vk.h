// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_GRAPHICS_QUEUE_VK_H_
#define FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_GRAPHICS_QUEUE_VK_H_

#include "impeller/renderer/graphics_queue.h"

namespace impeller {

class ContextVK;

class GraphicsQueueVK : public GraphicsQueue {
 public:
  explicit GraphicsQueueVK(const std::weak_ptr<ContextVK>& context);

  ~GraphicsQueueVK() override {}

  fml::Status Submit(const std::vector<std::shared_ptr<CommandBuffer>>& buffers,
                     const CompletionCallback& callback = {}) override;

  private:
    std::weak_ptr<ContextVK> context_;
};

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_RENDERER_BACKEND_VULKAN_GRAPHICS_QUEUE_VK_H_
