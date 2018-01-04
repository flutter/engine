// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/platform_view_ios.h"

#import <QuartzCore/CAEAGLLayer.h>

#include <utility>

#include "flutter/common/task_runners.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/common/io_manager.h"
#include "flutter/shell/gpu/gpu_rasterizer.h"
#include "flutter/shell/platform/darwin/common/process_info_mac.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/vsync_waiter_ios.h"
#include "flutter/shell/platform/darwin/ios/ios_external_texture_gl.h"
#include "lib/fxl/synchronization/waitable_event.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"

namespace shell {

PlatformViewIOS::PlatformViewIOS(PlatformView::Delegate& delegate,
                                 blink::TaskRunners task_runners,
                                 std::unique_ptr<IOSSurface> surface,
                                 NSObject<FlutterBinaryMessenger>* binaryMessenger)
    : PlatformView(delegate, std::move(task_runners)),
      ios_surface_(std::move(surface)),
      binary_messenger_(binaryMessenger) {
  FXL_DCHECK(ios_surface_ != nullptr);
}

PlatformViewIOS::~PlatformViewIOS() = default;

void PlatformViewIOS::ToggleAccessibility(UIView* view, bool enabled) {
  if (enabled) {
    if (!accessibility_bridge_) {
      accessibility_bridge_.reset(new shell::AccessibilityBridge(view, this));
    }
  } else {
    accessibility_bridge_ = nullptr;
  }
  SetSemanticsEnabled(enabled);
}

void PlatformViewIOS::UpdateSemantics(std::vector<blink::SemanticsNode> update) {
  if (accessibility_bridge_)
    accessibility_bridge_->UpdateSemantics(std::move(update));
}

void PlatformViewIOS::HandlePlatformMessage(fxl::RefPtr<blink::PlatformMessage> message) {
  platform_message_router_.HandlePlatformMessage(std::move(message));
}

void PlatformViewIOS::RegisterExternalTexture(int64_t texture_id,
                                              NSObject<FlutterTexture>* texture) {
  RegisterTexture(std::make_shared<IOSExternalTextureGL>(texture_id, texture));
}

std::unique_ptr<Surface> PlatformViewIOS::CreateRenderingSurface() {
  return ios_surface_->CreateGPUSurface();
}

sk_sp<GrContext> PlatformViewIOS::CreateResourceContext() const {
  if (!ios_surface_->CreateResourceMakeCurrent()) {
    FLX_LOG(INFO) << "Could not make resource context current on IO thread. Async texture uploads "
                     "will be disabled.";
    return nullptr;
  }

  return IOManager::CreateCompatibleResourceLoadingContext(
      GrBackend::kOpenGL_GrBackend,
      reinterpret_cast<GrBackendContext>(GrGLCreateNativeInterface()));
}

}  // namespace shell
