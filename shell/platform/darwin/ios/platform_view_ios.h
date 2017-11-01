// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_PLATFORM_VIEW_IOS_H_
#define SHELL_PLATFORM_IOS_PLATFORM_VIEW_IOS_H_

#include <memory>

#include "flutter/fml/memory/weak_ptr.h"
#include "flutter/shell/common/platform_view.h"
#include "flutter/shell/platform/darwin/ios/framework/Headers/FlutterTexture.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/accessibility_bridge.h"
#include "flutter/shell/platform/darwin/ios/framework/Source/platform_message_router.h"
#include "flutter/shell/platform/darwin/ios/ios_surface.h"
#include "lib/fxl/functional/closure.h"
#include "lib/fxl/macros.h"

namespace shell {

class PlatformViewIOS : public PlatformView {
 public:
  explicit PlatformViewIOS(PlatformView::Delegate& delegate,
                           blink::TaskRunners task_runners,
                           std::unique_ptr<IOSSurface> surface,
                           NSObject<FlutterBinaryMessenger>* binaryMessenger);

  ~PlatformViewIOS() override;

  void ToggleAccessibility(UIView* view, bool enabled);

  PlatformMessageRouter& platform_message_router() {
    return platform_message_router_;
  }

  void HandlePlatformMessage(
      fxl::RefPtr<blink::PlatformMessage> message) override;

  void RegisterExternalTexture(int64_t id, NSObject<FlutterTexture>* texture);

  void UpdateSemantics(std::vector<blink::SemanticsNode> update) override;

  NSObject<FlutterBinaryMessenger>* binary_messenger() const {
    return binary_messenger_;
  }

 private:
  std::unique_ptr<IOSSurface> ios_surface_;
  PlatformMessageRouter platform_message_router_;
  std::unique_ptr<AccessibilityBridge> accessibility_bridge_;
  fxl::Closure firstFrameCallback_;
  NSObject<FlutterBinaryMessenger>* binary_messenger_;

  // |shell::PlatformView|
  std::unique_ptr<Surface> CreateRenderingSurface() override;

  // |shell::PlatformView|
  sk_sp<GrContext> CreateResourceContext() const override;

  FXL_DISALLOW_COPY_AND_ASSIGN(PlatformViewIOS);
};

}  // namespace shell

#endif  // SHELL_PLATFORM_IOS_PLATFORM_VIEW_IOS_H_
