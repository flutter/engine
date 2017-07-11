// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_CONTENT_HANDLER_VULKAN_SURFACE_PRODUCER_H_
#define FLUTTER_CONTENT_HANDLER_VULKAN_SURFACE_PRODUCER_H_

#include "apps/mozart/lib/scene/client/resources.h"
#include "apps/mozart/lib/scene/client/session.h"
#include "flutter/flow/scene_update_context.h"
#include "flutter/vulkan/vulkan_application.h"
#include "flutter/vulkan/vulkan_device.h"
#include "flutter/vulkan/vulkan_proc_table.h"
#include "lib/ftl/macros.h"
#include "lib/mtl/tasks/message_loop.h"
#include "third_party/skia/include/gpu/vk/GrVkBackendContext.h"

namespace flutter_runner {

class VulkanSurfaceProducer : public flow::SceneUpdateContext::SurfaceProducer {
 public:
  VulkanSurfaceProducer();

  // |flow::SceneUpdateContext::SurfaceProducer|
  ~VulkanSurfaceProducer() override;

  // |flow::SceneUpdateContext::SurfaceProducer|
  sk_sp<SkSurface> ProduceSurface(SkISize size,
                                  mozart::client::Session* session,
                                  uint32_t& session_image_id,
                                  mx::event& acquire_release,
                                  mx::event& release_fence) override;

  void Tick();

  bool FinishFrame();

  bool IsValid() const { return valid_; }

 private:
  void RecycleBuffers();

  struct Surface {
    sk_sp<GrVkBackendContext> backend_context;
    sk_sp<SkSurface> sk_surface;
    mx::vmo vmo;
    VkImage vk_image;
    VkDeviceMemory vk_memory;
    std::unique_ptr<mozart::client::Image> session_image;

    Surface(sk_sp<GrVkBackendContext> backend_context,
            sk_sp<SkSurface> p_sk_surface,
            mx::vmo p_vmo,
            VkImage vk_image,
            VkDeviceMemory vk_memory,
            mozart::client::Session* session)
        : backend_context(std::move(backend_context)),
          sk_surface(std::move(p_sk_surface)),
          vmo(std::move(p_vmo)),
          vk_image(vk_image),
          vk_memory(vk_memory) {
      // Duplicate the underlying VMO. This is the VMO we are going to pass onto
      // the session.
      mx::vmo session_vmo;
      mx_status_t status = vmo.duplicate(MX_RIGHT_SAME_RIGHTS, &session_vmo);
      FTL_CHECK(status == MX_OK);
      mozart::client::Memory memory(session, std::move(session_vmo),
                                    mozart2::MemoryType::VK_DEVICE_MEMORY);
      auto image_info = mozart2::ImageInfo::New();
      image_info->width = sk_surface->width();
      image_info->height = sk_surface->height();
      image_info->stride = 4 * sk_surface->width();
      image_info->pixel_format = mozart2::ImageInfo::PixelFormat::BGRA_8;
      image_info->color_space = mozart2::ImageInfo::ColorSpace::SRGB;
      image_info->tiling = mozart2::ImageInfo::Tiling::LINEAR;
      session_image = std::make_unique<mozart::client::Image>(
          memory, 0 /* memory offset */, std::move(image_info));
    }

    ~Surface() {
      session_image.reset();
      FTL_DCHECK(backend_context);
      vkFreeMemory(backend_context->fDevice, vk_memory, NULL);
      vkDestroyImage(backend_context->fDevice, vk_image, NULL);
    }
  };

  std::unique_ptr<Surface> CreateSurface(mozart::client::Session* session,
                                         uint32_t width,
                                         uint32_t height);

  struct Swapchain {
    std::queue<std::unique_ptr<Surface>> queue;
    uint32_t tick_count = 0;
    static constexpr uint32_t kMaxSurfaces = 3;
    static constexpr uint32_t kMaxTickBeforeDiscard = 3;
  };

  using size_key_t = uint64_t;
  static size_key_t MakeSizeKey(uint32_t width, uint32_t height) {
    return (static_cast<uint64_t>(width) << 32) | static_cast<uint64_t>(height);
  }

  // Note: the order here is very important. The proctable bust be destroyed
  // last because it contains the function pointers for VkDestroyDevice and
  // VkDestroyInstance. The backend context owns the VkDevice and the
  // VkInstance, so it must be destroyed after the logical device and the
  // application, which own other vulkan objects associated with the device
  // and instance
  ftl::RefPtr<vulkan::VulkanProcTable> vk_;
  sk_sp<GrVkBackendContext> backend_context_;
  std::unique_ptr<vulkan::VulkanDevice> logical_device_;
  std::unique_ptr<vulkan::VulkanApplication> application_;
  sk_sp<GrContext> context_;

  // These three containers hold surfaces in various stages of recycling

  // Buffers exist in available_surfaces_ when they are ready to be recycled
  // ProduceSurface will look here for an appropriately sized surface before
  // creating a new one
  // The Swapchain's tick_count is incremented in Tick and decremented when
  // a surface is taken from the queue, when the tick count goes above
  // kMaxTickBeforeDiscard the Swapchain is discarded. Newly surfaces are
  // added to the queue iff there aar less than kMaxSurfaces already in the
  // queue
  std::unordered_map<size_key_t, Swapchain> available_surfaces_;

  struct PendingSurfaceInfo {
    mtl::MessageLoop::HandlerKey handler_key;
    std::unique_ptr<Surface> surface;
  };
  // Surfaces produced by ProduceSurface live in outstanding_surfaces_ until
  // FinishFrame is called, at which point they are moved to pending_surfaces_
  std::vector<PendingSurfaceInfo> outstanding_surfaces_;
  // Surfaces exist in pending surfaces until they are released by the buffer
  // consumer
  std::unordered_map<mtl::MessageLoop::HandlerKey /* TODO: Unused remove */,
                     PendingSurfaceInfo>
      pending_surfaces_;
  mtl::MessageLoop::HandlerKey dummy_handler_key_ = 1;
  bool valid_;

  bool Initialize();

  FTL_DISALLOW_COPY_AND_ASSIGN(VulkanSurfaceProducer);
};

}  // namespace flutter_runner

#endif  // FLUTTER_CONTENT_HANDLER_VULKAN_SURFACE_PRODUCER_H_
