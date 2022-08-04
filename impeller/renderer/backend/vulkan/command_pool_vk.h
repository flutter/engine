#pragma once

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/backend/vulkan/swapchain_vk.h"
#include "impeller/renderer/backend/vulkan/vk.h"

namespace impeller {

class CommandPoolVK {
 public:
  static std::shared_ptr<CommandPoolVK> Create(vk::Device device,
                                               uint32_t queue_index);

  explicit CommandPoolVK(vk::UniqueCommandPool command_pool)
      : command_pool_(std::move(command_pool)) {}

  ~CommandPoolVK() = default;

  vk::UniqueCommandPool command_pool_;
};

}  // namespace impeller
