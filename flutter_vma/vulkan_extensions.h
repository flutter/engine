#pragma once

#include <vector>

#include "third_party/skia/include/core/SkString.h"
#include "third_party/skia/include/gpu/vk/GrVkBackendContext.h"

namespace flutter {

/**
 * Helper class that eats in an array of extensions strings for instance and
 * device and allows for quicker querying if an extension is present.
 */
class VulkanExtensions {
 public:
  VulkanExtensions() {}

  void init(skgpu::VulkanGetProc,
            VkInstance,
            VkPhysicalDevice,
            uint32_t instanceExtensionCount,
            const char* const* instanceExtensions,
            uint32_t deviceExtensionCount,
            const char* const* deviceExtensions);

  bool hasExtension(const char[], uint32_t minVersion) const;

  struct Info {
    Info() {}
    explicit Info(const char* name) : fName(name), fSpecVersion(0) {}

    SkString fName;
    uint32_t fSpecVersion;

    struct Less {
      bool operator()(const Info& a, const SkString& b) const {
        return strcmp(a.fName.c_str(), b.c_str()) < 0;
      }
      bool operator()(const SkString& a,
                      const VulkanExtensions::Info& b) const {
        return strcmp(a.c_str(), b.fName.c_str()) < 0;
      }
    };
  };

 private:
  void getSpecVersions(skgpu::VulkanGetProc getProc,
                       VkInstance,
                       VkPhysicalDevice);

  std::vector<Info> extensions_;
};

}  // namespace flutter
