#include "impeller/renderer/backend/vulkan/command_pool_vk.h"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {
std::shared_ptr<CommandPoolVK> CommandPoolVK::Create(vk::Device device,
                                                     uint32_t queue_index) {
  vk::CommandPoolCreateInfo create_info;
  create_info.setQueueFamilyIndex(queue_index);

  auto res = device.createCommandPoolUnique(create_info);
  if (res.result != vk::Result::eSuccess) {
    FML_CHECK(false) << "Failed to create command pool: "
                     << vk::to_string(res.result);
    return nullptr;
  }

  return std::make_shared<CommandPoolVK>(std::move(res.value));
}
}  // namespace impeller
