#include <iostream>

#include "flutter/fml/concurrent_message_loop.h"
#include "flutter/fml/paths.h"
#include "impeller/entity/vk/entity_shaders_vk.h"
#include "impeller/entity/vk/framebuffer_blend_shaders_vk.h"
#include "impeller/entity/vk/modern_shaders_vk.h"
#include "impeller/renderer/backend/metal/context_mtl.h"
#include "impeller/renderer/backend/vulkan/context_vk.h"
#include "impeller/renderer/vk/compute_shaders_vk.h"
#include "impeller/scene/shaders/vk/scene_shaders_vk.h"

#include "flutter/vulkan/procs/vulkan_proc_table.h"

// static std::vector<std::shared_ptr<fml::Mapping>> ShaderLibraryMappings() {
//   return {std::make_shared<fml::NonOwnedMapping>(
//               impeller_entity_shaders_data, impeller_entity_shaders_length),
//           std::make_shared<fml::NonOwnedMapping>(
//               impeller_modern_shaders_data, impeller_modern_shaders_length),
//           std::make_shared<fml::NonOwnedMapping>(
//               impeller_framebuffer_blend_shaders_data,
//               impeller_framebuffer_blend_shaders_length),
//           std::make_shared<fml::NonOwnedMapping>(impeller_scene_shaders_data,
//                                                  impeller_scene_shaders_length),
//           std::make_shared<fml::NonOwnedMapping>(
//               impeller_compute_shaders_data,
//               impeller_compute_shaders_length)};
// }

static std::vector<std::shared_ptr<fml::Mapping>> ShaderLibraryMappings() {
  return {
      std::make_shared<fml::NonOwnedMapping>(impeller_entity_shaders_vk_data,
                                             impeller_entity_shaders_vk_length),
      std::make_shared<fml::NonOwnedMapping>(impeller_modern_shaders_vk_data,
                                             impeller_modern_shaders_vk_length),
      std::make_shared<fml::NonOwnedMapping>(
          impeller_framebuffer_blend_shaders_vk_data,
          impeller_framebuffer_blend_shaders_vk_length),
      std::make_shared<fml::NonOwnedMapping>(impeller_scene_shaders_vk_data,
                                             impeller_scene_shaders_vk_length),
      std::make_shared<fml::NonOwnedMapping>(
          impeller_compute_shaders_vk_data,
          impeller_compute_shaders_vk_length)};
}

int main() {
  auto concurrent_loop = fml::ConcurrentMessageLoop::Create();
  auto proc_table = fml::MakeRefCounted<vulkan::VulkanProcTable>();

  impeller::ContextVK::Settings context_settings;
  context_settings.proc_address_callback =
      proc_table->NativeGetInstanceProcAddr();
  context_settings.shader_libraries_data = ShaderLibraryMappings();
  context_settings.cache_directory = fml::paths::GetCachesDirectory();
  context_settings.worker_task_runner = concurrent_loop->GetTaskRunner();
  context_settings.enable_validation = false;
  auto context = impeller::ContextVK::Create(std::move(context_settings));
  // auto context =
  //     impeller::ContextMTL::Create(ShaderLibraryMappings(), "gpu_model");

  if (!context) {
    std::cerr << "Failed to create context." << std::endl;
    return -1;
  }

  std::cout << context->DescribeGpuModel() << std::endl;
  return 0;
}