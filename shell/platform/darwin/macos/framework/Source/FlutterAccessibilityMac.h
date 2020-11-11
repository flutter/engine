// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FlutterAccessibilityMac_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FlutterAccessibilityMac_H_

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/embedder/embedder.h"
#include "flutter/third_party/accessibility/flutter_accessibility.h"

#define kInvalidID 0
#define kRootNode 0

namespace ax {

class FlutterAccessibilityMac : public FlutterAccessibility {
 public:
  FlutterAccessibilityMac();
  ~FlutterAccessibilityMac();
  // FlutterAccessibility override
  void Init(AccessibilityBridge* bridge, AXNode* node) override;
  void OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event) override;
  const AXNodeData& GetData() const override;
  gfx::NativeViewAccessible GetNativeViewAccessible() override;
  gfx::NativeViewAccessible GetParent() override;
  int GetChildCount() const override;
  SkRect GetBoundsRect(const AXCoordinateSystem coordinate_system,
                          const AXClippingBehavior clipping_behavior,
                          AXOffscreenResult* offscreen_result) const override;
  gfx::NativeViewAccessible ChildAtIndex(int index) override;
  gfx::NativeViewAccessible GetNSWindow() override;
 private:
  AccessibilityBridge* bridge_;
  AXNode* ax_node_;
  AXPlatformNode* ax_platform_node_;
};

} // namespace ax

#endif
