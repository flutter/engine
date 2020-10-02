// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_AXPLATFORMNODEDELEGATEMAC_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_AXPLATFORMNODEDELEGATEMAC_H_

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/embedder/embedder.h"
#include "flutter/third_party/accessibility/ax/platform/ax_platform_node_delegate_base.h"

#define kInvalidID 0
#define kRootNode 0

namespace flutter {

class AXPlatformNodeDelegateMac : public ax::AXPlatformNodeDelegateBase {
 public:
  class AXPlatformNodeDelegateOwner {
   public:
    virtual ~AXPlatformNodeDelegateOwner() = default;
    virtual AXPlatformNodeDelegateMac* GetOrCreateAXPlatformNodeDelegate(int32_t id) = 0;
    virtual const SkMatrix GetWindowTransform() = 0;
  };
  AXPlatformNodeDelegateMac(AXPlatformNodeDelegateOwner* bridge);
  ~AXPlatformNodeDelegateMac();
  void UpdateWith(const FlutterSemanticsNode* flutter_node);
  const std::vector<int32_t>& GetChildren();
  // AXPlatformNodeDelegateBase override
  const ax::AXNodeData& GetData() const override;
  gfx::NativeViewAccessible GetNativeViewAccessible() override;
  gfx::NativeViewAccessible GetParent() override;
  int GetChildCount() const override;
  SkRect GetBoundsRect(const ax::AXCoordinateSystem coordinate_system,
                          const ax::AXClippingBehavior clipping_behavior,
                          ax::AXOffscreenResult* offscreen_result) const override;
  gfx::NativeViewAccessible ChildAtIndex(int index) override;
 private:
  AXPlatformNodeDelegateOwner* bridge_;
  ax::AXNodeData node_data_;
  ax::AXPlatformNode* ax_platform_node_;
  int32_t parent_ = kInvalidID;
  std::vector<int32_t> children_in_traversal_order_;
};

} // namespace flutter

#endif
