#ifndef FLUTTER_SHELL_GPU_GPU_SURFACE_VULKAN_DELEGATE_H_
#define FLUTTER_SHELL_GPU_GPU_SURFACE_VULKAN_DELEGATE_H_

#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/shell/gpu/gpu_surface_delegate.h"
#include "flutter/vulkan/vulkan_proc_table.h"

namespace flutter {

class GPUSurfaceVulkanDelegate : public GPUSurfaceDelegate {
 public:
  ~GPUSurfaceVulkanDelegate() override;

  // Obtain a reference to the Vulkan implementation's proc table.
  virtual fml::RefPtr<vulkan::VulkanProcTable> vk() = 0;

  // |GPUSurfaceDelegate|
  ExternalViewEmbedder* GetExternalViewEmbedder() override;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_GPU_GPU_SURFACE_VULKAN_DELEGATE_H_
