// Copyright 2017 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <async/cpp/wait.h>
#include <zx/event.h>
#include <zx/vmo.h>

#include <memory>

#include "flutter/flow/scene_update_context.h"
#include "flutter/vulkan/vulkan_handle.h"
#include "flutter/vulkan/vulkan_proc_table.h"
#include "lib/fxl/macros.h"
#include "lib/ui/scenic/client/resources.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/gpu/vk/GrVkBackendContext.h"

namespace flutter_runner {

class VulkanSurface : public flow::SceneUpdateContext::SurfaceProducerSurface {
 public:
  VulkanSurface(vulkan::VulkanProcTable& p_vk,
                sk_sp<GrContext> context,
                sk_sp<GrVkBackendContext> backend_context,
                scenic_lib::Session* session,
                const SkISize& size);

  ~VulkanSurface() override;

  size_t AdvanceAndGetAge() override;

  bool FlushSessionAcquireAndReleaseEvents() override;

  bool IsValid() const override;

  SkISize GetSize() const override;

  // Note: It is safe for the caller to collect the surface in the
  // |on_writes_committed| callback.
  void SignalWritesFinished(
      std::function<void(void)> on_writes_committed) override;

  // |flow::SceneUpdateContext::SurfaceProducerSurface|
  scenic_lib::Image* GetImage() override;

  // |flow::SceneUpdateContext::SurfaceProducerSurface|
  sk_sp<SkSurface> GetSkiaSurface() const override;

  // This transfers ownership of the GrBackendSemaphore but not the underlying
  // VkSemaphore (i.e. it is ok to let the returned GrBackendSemaphore go out of
  // scope but it is not ok to call VkDestroySemaphore on the underlying
  // VkSemaphore)
  GrBackendSemaphore GetAcquireSemaphore() const;

 private:
  async_wait_result_t OnHandleReady(async_t* async,
                                    zx_status_t status,
                                    const zx_packet_signal_t* signal);

  bool AllocateDeviceMemory(sk_sp<GrContext> context,
                            const SkISize& size,
                            zx::vmo& exported_vmo);

  bool SetupSkiaSurface(sk_sp<GrContext> context,
                        const SkISize& size,
                        const VkImageCreateInfo& image_create_info,
                        const VkMemoryRequirements& memory_reqs);

  bool CreateFences();

  bool PushSessionImageSetupOps(scenic_lib::Session* session,
                                zx::vmo exported_vmo);

  void Reset();

  vulkan::VulkanHandle<VkSemaphore> SemaphoreFromEvent(
      const zx::event& event) const;

  vulkan::VulkanProcTable& vk_;
  sk_sp<GrVkBackendContext> backend_context_;
  scenic_lib::Session* session_;
  vulkan::VulkanHandle<VkImage> vk_image_;
  vulkan::VulkanHandle<VkDeviceMemory> vk_memory_;
  sk_sp<SkSurface> sk_surface_;
  std::unique_ptr<scenic_lib::Image> session_image_;
  zx::event acquire_event_;
  vulkan::VulkanHandle<VkSemaphore> acquire_semaphore_;
  zx::event release_event_;
  async_t* async_;
  async::WaitMethod<VulkanSurface, &VulkanSurface::OnHandleReady> wait_;
  std::function<void()> pending_on_writes_committed_;
  size_t age_ = 0;
  bool valid_ = false;

  FXL_DISALLOW_COPY_AND_ASSIGN(VulkanSurface);
};

}  // namespace flutter_runner
