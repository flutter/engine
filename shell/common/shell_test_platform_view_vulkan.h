// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_SHELL_TEST_PLATFORM_VIEW_VULKAN_H_
#define FLUTTER_SHELL_COMMON_SHELL_TEST_PLATFORM_VIEW_VULKAN_H_

#include "flutter/shell/common/shell_test_external_view_embedder.h"
#include "flutter/shell/common/shell_test_platform_view.h"
#include "flutter/shell/gpu/gpu_surface_vulkan_delegate.h"
#include "flutter/vulkan/vulkan_application.h"
#include "flutter/vulkan/vulkan_device.h"
#include "flutter/vulkan/vulkan_skia_proc_table.h"

namespace flutter {
namespace testing {

class ShellTestPlatformViewVulkan : public ShellTestPlatformView {
 public:
  ShellTestPlatformViewVulkan(PlatformView::Delegate& delegate,
                              const TaskRunners& task_runners,
                              std::shared_ptr<ShellTestVsyncClock> vsync_clock,
                              CreateVsyncWaiter create_vsync_waiter,
                              std::shared_ptr<ShellTestExternalViewEmbedder>
                                  shell_test_external_view_embedder);

  ~ShellTestPlatformViewVulkan() override;

  void SimulateVSync() override;

 private:
  class OffScreenContext {
   public:
    OffScreenContext(fml::RefPtr<vulkan::VulkanProcTable> vk);

    ~OffScreenContext();

    sk_sp<GrDirectContext> GetContext() { return context_; }

   private:
    fml::RefPtr<vulkan::VulkanProcTable> vk_;
    std::unique_ptr<vulkan::VulkanApplication> application_;
    std::unique_ptr<vulkan::VulkanDevice> logical_device_;
    sk_sp<skgpu::VulkanMemoryAllocator> memory_allocator_;
    sk_sp<GrDirectContext> context_;

    bool CreateSkiaGrContext();
    bool CreateSkiaBackendContext(GrVkBackendContext* context);

    FML_DISALLOW_COPY_AND_ASSIGN(OffScreenContext);
  };

  class OffScreenStudio : public flutter::Studio {
   public:
    //------------------------------------------------------------------------------
    /// @brief      Create a GPUStudioVulkan while letting it reuse an existing
    ///             GrDirectContext.
    ///
    OffScreenStudio(const std::shared_ptr<OffScreenContext>& offscreen_context)
        : offscreen_context_(offscreen_context) {}

    ~OffScreenStudio() override = default;

    // |Studio|
    bool IsValid() override { return GetContext(); }

    // |Studio|
    GrDirectContext* GetContext() override {
      return offscreen_context_->GetContext().get();
    }

   private:
    std::shared_ptr<OffScreenContext> offscreen_context_;

    FML_DISALLOW_COPY_AND_ASSIGN(OffScreenStudio);
  };

  class OffScreenSurface : public flutter::Surface {
   public:
    OffScreenSurface(const std::shared_ptr<OffScreenContext>& offscreen_context,
                     std::shared_ptr<ShellTestExternalViewEmbedder>
                         shell_test_external_view_embedder);

    ~OffScreenSurface() override;

    // |Surface|
    bool IsValid() override;

    // |Surface|
    std::unique_ptr<SurfaceFrame> AcquireFrame(int64_t view_id,
                                               const SkISize& size) override;

    // |Surface|
    SkMatrix GetRootTransformation() const override;

    // |Surface|
    GrDirectContext* GetContext() override;

   private:
    std::shared_ptr<ShellTestExternalViewEmbedder>
        shell_test_external_view_embedder_;
    std::shared_ptr<OffScreenContext> offscreen_context_;

    FML_DISALLOW_COPY_AND_ASSIGN(OffScreenSurface);
  };

  CreateVsyncWaiter create_vsync_waiter_;

  std::shared_ptr<ShellTestVsyncClock> vsync_clock_;

  fml::RefPtr<vulkan::VulkanProcTable> proc_table_;

  std::shared_ptr<ShellTestExternalViewEmbedder>
      shell_test_external_view_embedder_;

  std::shared_ptr<OffScreenContext> offscreen_context_;

  // |PlatformView|
  std::unique_ptr<Studio> CreateRenderingStudio() override;

  // |PlatformView|
  std::unique_ptr<Surface> CreateRenderingSurface(int64_t view_id) override;

  // |PlatformView|
  std::shared_ptr<ExternalViewEmbedder> CreateExternalViewEmbedder() override;

  // |PlatformView|
  std::unique_ptr<VsyncWaiter> CreateVSyncWaiter() override;

  // |PlatformView|
  PointerDataDispatcherMaker GetDispatcherMaker() override;

  FML_DISALLOW_COPY_AND_ASSIGN(ShellTestPlatformViewVulkan);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_SHELL_TEST_PLATFORM_VIEW_VULKAN_H_
