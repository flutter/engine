// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_platform_view.h"

namespace flutter {

#if SHELL_ENABLE_GL
EmbedderPlatformView::EmbedderPlatformView(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    EmbedderSurfaceGL::GLDispatchTable gl_dispatch_table,
    bool fbo_reset_after_present,
    PlatformDispatchTable platform_dispatch_table,
    std::unique_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, std::move(task_runners)),
      embedder_surface_(std::make_unique<EmbedderSurfaceGL>(
          gl_dispatch_table,
          fbo_reset_after_present,
          std::move(external_view_embedder))),
      platform_dispatch_table_(platform_dispatch_table) {}
#endif

#if SHELL_ENABLE_VULKAN
EmbedderPlatformView::EmbedderPlatformView(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    EmbedderSurfaceVulkan::VulkanDispatchTable vulkan_dispatch_table,
    PlatformDispatchTable platform_dispatch_table,
    std::unique_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, std::move(task_runners)),
      embedder_surface_(std::make_unique<EmbedderSurfaceVulkan>(
          vulkan_dispatch_table,
          std::move(external_view_embedder))),
      platform_dispatch_table_(platform_dispatch_table) {}
#endif

EmbedderPlatformView::EmbedderPlatformView(
    PlatformView::Delegate& delegate,
    flutter::TaskRunners task_runners,
    EmbedderSurfaceSoftware::SoftwareDispatchTable software_dispatch_table,
    PlatformDispatchTable platform_dispatch_table,
    std::unique_ptr<EmbedderExternalViewEmbedder> external_view_embedder)
    : PlatformView(delegate, std::move(task_runners)),
      embedder_surface_(std::make_unique<EmbedderSurfaceSoftware>(
          software_dispatch_table,
          std::move(external_view_embedder))),
      platform_dispatch_table_(platform_dispatch_table) {}

EmbedderPlatformView::~EmbedderPlatformView() = default;

void EmbedderPlatformView::UpdateSemantics(
    flutter::SemanticsNodeUpdates update,
    flutter::CustomAccessibilityActionUpdates actions) {
  if (platform_dispatch_table_.update_semantics_nodes_callback != nullptr) {
    platform_dispatch_table_.update_semantics_nodes_callback(std::move(update));
  }
  if (platform_dispatch_table_.update_semantics_custom_actions_callback !=
      nullptr) {
    platform_dispatch_table_.update_semantics_custom_actions_callback(
        std::move(actions));
  }
}

void EmbedderPlatformView::HandlePlatformMessage(
    fml::RefPtr<flutter::PlatformMessage> message) {
  if (!message) {
    return;
  }

  if (platform_dispatch_table_.platform_message_response_callback == nullptr) {
    if (message->response()) {
      message->response()->CompleteEmpty();
    }
    return;
  }

  platform_dispatch_table_.platform_message_response_callback(
      std::move(message));
}

std::unique_ptr<Surface> EmbedderPlatformView::CreateRenderingSurface() {
  if (embedder_surface_ == nullptr) {
    FML_LOG(ERROR) << "Embedder surface was null.";
    return nullptr;
  }

  return embedder_surface_->CreateGPUSurface();
}

sk_sp<GrContext> EmbedderPlatformView::CreateResourceContext() const {
  if (embedder_surface_ == nullptr) {
    FML_LOG(ERROR) << "Embedder surface was null.";
    return nullptr;
  }
  return embedder_surface_->CreateResourceContext();
}

std::unique_ptr<VsyncWaiter> EmbedderPlatformView::CreateVSyncWaiter() {
  if (!platform_dispatch_table_.vsync_callback) {
    // Superclass implementation creates a timer based fallback.
    return PlatformView::CreateVSyncWaiter();
  }

  return std::make_unique<EmbedderVsyncWaiter>(
      platform_dispatch_table_.vsync_callback, task_runners_);
}

}  // namespace flutter
