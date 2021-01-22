// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterPlatformNodeDelegateMac.h"

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterAppDelegate.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"

#include "flutter/shell/platform/common/accessibility_bridge.h"
#include "flutter/third_party/accessibility/ax/ax_action_data.h"
#include "flutter/third_party/accessibility/ax/ax_node_position.h"
#include "flutter/third_party/accessibility/ax/platform/ax_platform_node.h"
#include "flutter/third_party/accessibility/ax/platform/ax_platform_node_base.h"
#include "flutter/third_party/accessibility/base/string_utils.h"
#include "flutter/third_party/accessibility/gfx/geometry/rect_conversions.h"

namespace flutter {  // namespace

FlutterPlatformNodeDelegateMac::FlutterPlatformNodeDelegateMac(FlutterEngine* engine)
    : engine_(engine) {
  ax_platform_node_ = ui::AXPlatformNode::Create(this);
  NSCAssert(ax_platform_node_, @"Failed to create platform node.");
}

FlutterPlatformNodeDelegateMac::~FlutterPlatformNodeDelegateMac() {
  ax_platform_node_->Destroy();
}

gfx::NativeViewAccessible FlutterPlatformNodeDelegateMac::GetNativeViewAccessible() {
  NSCAssert(ax_platform_node_, @"Platform node does not exist.");
  return ax_platform_node_->GetNativeViewAccessible();
}

gfx::NativeViewAccessible FlutterPlatformNodeDelegateMac::GetParent() {
  gfx::NativeViewAccessible parent = FlutterPlatformNodeDelegate::GetParent();
  if (!parent) {
    NSCAssert(engine_, @"Flutter engine should not be deallocated");
    return engine_.viewController.view;
  }
  return parent;
}

gfx::Rect FlutterPlatformNodeDelegateMac::GetBoundsRect(
    const ui::AXCoordinateSystem coordinate_system,
    const ui::AXClippingBehavior clipping_behavior,
    ui::AXOffscreenResult* offscreen_result) const {
  gfx::Rect bounds = FlutterPlatformNodeDelegate::GetBoundsRect(
      coordinate_system, clipping_behavior, offscreen_result);
  // Applies window transform.
  NSRect local_bounds;
  local_bounds.origin.x = bounds.x();
  local_bounds.origin.y = bounds.y();
  local_bounds.size.width = bounds.width();
  local_bounds.size.height = bounds.height();
  // local_bounds is flipped against y axis.
  local_bounds.origin.y = -local_bounds.origin.y - local_bounds.size.height;
  __strong FlutterEngine* strong_engine = engine_;
  NSCAssert(strong_engine, @"Flutter engine should not be deallocated");
  NSRect view_bounds = [strong_engine.viewController.view convertRectFromBacking:local_bounds];
  NSRect window_bounds = [strong_engine.viewController.view convertRect:view_bounds toView:nil];
  NSRect global_bounds =
      [[strong_engine.viewController.view window] convertRectToScreen:window_bounds];
  // The voiceover seems to only accept bounds that are relative to primary screen.
  // Thus, we use [[NSScreen screens] firstObject] instead of [NSScreen mainScreen].
  NSScreen* screen = [[NSScreen screens] firstObject];
  NSRect screen_bounds = [screen frame];
  // The screen is flipped against y axis.
  global_bounds.origin.y =
      screen_bounds.size.height - global_bounds.origin.y - global_bounds.size.height;
  gfx::RectF result(global_bounds.origin.x, global_bounds.origin.y, global_bounds.size.width,
                    global_bounds.size.height);
  return gfx::ToEnclosingRect(result);
}

gfx::NativeViewAccessible FlutterPlatformNodeDelegateMac::GetNSWindow() {
  FlutterAppDelegate* appDelegate = (FlutterAppDelegate*)[NSApp delegate];
  return appDelegate.mainFlutterWindow;
}

std::string FlutterPlatformNodeDelegateMac::GetLiveRegionText() const {
  if (GetAXNode()->IsIgnored()) {
    return "";
  }

  std::string text = GetData().GetStringAttribute(ax::mojom::StringAttribute::kName);
  if (!text.empty()) {
    return text;
  };
  NSCAssert(engine_, @"Flutter engine should not be deallocated");
  auto bridge_ptr = engine_.accessibilityBridge.lock();
  NSCAssert(bridge_ptr, @"Accessibility bridge in flutter engine must not be null.");
  for (int32_t child : GetData().child_ids) {
    auto delegate_child = bridge_ptr->GetFlutterPlatformNodeDelegateFromID(child).lock();
    if (!delegate_child) {
      continue;
    }
    text += std::static_pointer_cast<FlutterPlatformNodeDelegateMac>(delegate_child)
                ->GetLiveRegionText();
  }
  return text;
}

}  // namespace flutter
