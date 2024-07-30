// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_SHELL_TEST_PLATFORM_VIEW_METAL_H_
#define FLUTTER_SHELL_COMMON_SHELL_TEST_PLATFORM_VIEW_METAL_H_

#include "flutter/fml/macros.h"
#include "flutter/shell/common/shell_test_platform_view.h"
#include "flutter/shell/surface/surface_metal_delegate.h"

namespace flutter {
namespace testing {

class DarwinContextMetal;

class ShellTestPlatformViewMetal final : public ShellTestPlatformView,
                                         public SurfaceMetalDelegate {
 public:
  ShellTestPlatformViewMetal(PlatformView::Delegate& delegate,
                             const TaskRunners& task_runners,
                             std::shared_ptr<ShellTestVsyncClock> vsync_clock,
                             CreateVsyncWaiter create_vsync_waiter,
                             std::shared_ptr<ShellTestExternalViewEmbedder>
                                 shell_test_external_view_embedder,
                             const std::shared_ptr<const fml::SyncSwitch>&
                                 is_gpu_disabled_sync_switch);

  // |ShellTestPlatformView|
  virtual ~ShellTestPlatformViewMetal() override;

 private:
  const std::unique_ptr<DarwinContextMetal> metal_context_;
  const CreateVsyncWaiter create_vsync_waiter_;
  const std::shared_ptr<ShellTestVsyncClock> vsync_clock_;
  const std::shared_ptr<ShellTestExternalViewEmbedder>
      shell_test_external_view_embedder_;

  // |ShellTestPlatformView|
  virtual void SimulateVSync() override;

  // |PlatformView|
  std::unique_ptr<VsyncWaiter> CreateVSyncWaiter() override;

  // |PlatformView|
  std::shared_ptr<ExternalViewEmbedder> CreateExternalViewEmbedder() override;

  // |PlatformView|
  PointerDataDispatcherMaker GetDispatcherMaker() override;

  // |PlatformView|
  std::unique_ptr<Surface> CreateRenderingSurface() override;

  // |PlatformView|
  std::shared_ptr<impeller::Context> GetImpellerContext() const override;

  // |SurfaceMetalDelegate|
  GPUCAMetalLayerHandle GetCAMetalLayer(
      const SkISize& frame_info) const override;

  // |SurfaceMetalDelegate|
  bool PresentDrawable(GrMTLHandle drawable) const override;

  // |SurfaceMetalDelegate|
  GPUMTLTextureInfo GetMTLTexture(const SkISize& frame_info) const override;

  // |SurfaceMetalDelegate|
  bool PresentTexture(GPUMTLTextureInfo texture) const override;

  FML_DISALLOW_COPY_AND_ASSIGN(ShellTestPlatformViewMetal);
};

}  // namespace testing
}  // namespace flutter

#endif  // FLUTTER_SHELL_COMMON_SHELL_TEST_PLATFORM_VIEW_METAL_H_
