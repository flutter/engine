// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/platform_view_ios.h"

#import <QuartzCore/CAEAGLLayer.h>

#include <utility>

#include "flutter/common/threads.h"
#include "flutter/fml/trace_event.h"
#include "flutter/shell/gpu/gpu_rasterizer.h"
#include "flutter/shell/platform/darwin/common/process_info_mac.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/vsync_waiter_ios.h"
#include "flutter/shell/platform/darwin/ios/ios_external_texture_gl.h"
#include "flutter/shell/platform/darwin/ios/ios_surface_gl.h"
#include "lib/fxl/synchronization/waitable_event.h"

namespace shell {

PlatformViewIOS::PlatformViewIOS(CALayer* layer, NSObject<FlutterBinaryMessenger>* binaryMessenger)
    : PlatformView(std::make_unique<GPURasterizer>(std::make_unique<ProcessInfoMac>())),
      layered_paint_context_(std::make_unique<IOSLayeredPaintContext>(surface_config_, layer)),
      weak_factory_(this),
      binary_messenger_(binaryMessenger) {}

PlatformViewIOS::~PlatformViewIOS() = default;

void PlatformViewIOS::Attach() {
  Attach(NULL);
}

void PlatformViewIOS::Attach(fxl::Closure firstFrameCallback) {
  CreateEngine();

  if (firstFrameCallback) {
    firstFrameCallback_ = firstFrameCallback;
    rasterizer_->AddNextFrameCallback([weakSelf = GetWeakPtr()] {
      if (weakSelf) {
        weakSelf->firstFrameCallback_();
        weakSelf->firstFrameCallback_ = nullptr;
      }
    });
  }
}

void PlatformViewIOS::NotifyCreated() {
  PlatformView::NotifyCreated(
    layered_paint_context_->rootIOSSurface()->CreateGPUSurface(), layered_paint_context_.get());
}

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

void PlatformViewIOS::SetupAndLoadFromSource(const std::string& assets_directory,
                                             const std::string& main,
                                             const std::string& packages) {
  blink::Threads::UI()->PostTask(
      [ engine = engine().GetWeakPtr(), assets_directory, main, packages ] {
        if (engine)
          engine->RunBundleAndSource(assets_directory, main, packages);
      });
}

void PlatformViewIOS::SetAssetBundlePathOnUI(const std::string& assets_directory) {
  blink::Threads::UI()->PostTask([ engine = engine().GetWeakPtr(), assets_directory ] {
    if (engine)
      engine->SetAssetBundlePath(assets_directory);
  });
}

fml::WeakPtr<PlatformViewIOS> PlatformViewIOS::GetWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

void PlatformViewIOS::UpdateSurfaceSize() {
  blink::Threads::Gpu()->PostTask([self = GetWeakPtr()]() {
    if (self && self->layered_paint_context_->rootIOSSurface()) {
      self->layered_paint_context_->rootIOSSurface()->UpdateStorageSizeIfNecessary();
    }
  });
}

VsyncWaiter* PlatformViewIOS::GetVsyncWaiter() {
  if (!vsync_waiter_) {
    vsync_waiter_ = std::make_unique<VsyncWaiterIOS>();
  }
  return vsync_waiter_.get();
}

bool PlatformViewIOS::ResourceContextMakeCurrent() {
  return layered_paint_context_->rootIOSSurface() ? layered_paint_context_->rootIOSSurface()->ResourceContextMakeCurrent() : false;
}

void PlatformViewIOS::UpdateSemantics(blink::SemanticsNodeUpdates update) {
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

void PlatformViewIOS::RegisterExternalLayer(int64_t texture_id,
                                            CALayer *layer) {
  RegisterTexture(std::make_shared<IOSExternalTextureLayer>(texture_id, layer));
}

void PlatformViewIOS::RunFromSource(const std::string& assets_directory,
                                    const std::string& main,
                                    const std::string& packages) {
  auto latch = new fxl::ManualResetWaitableEvent();

  dispatch_async(dispatch_get_main_queue(), ^{
    SetupAndLoadFromSource(assets_directory, main, packages);
    latch->Signal();
  });

  latch->Wait();
  delete latch;
}

void PlatformViewIOS::SetAssetBundlePath(const std::string& assets_directory) {
  auto latch = new fxl::ManualResetWaitableEvent();

  dispatch_async(dispatch_get_main_queue(), ^{
    SetAssetBundlePathOnUI(assets_directory);
    latch->Signal();
  });

  latch->Wait();
  delete latch;
}

}  // namespace shell
