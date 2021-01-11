// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_platform_node_delegate.h"

#include "flutter/third_party/accessibility/ax/ax_action_data.h"
#include "flutter/third_party/accessibility/gfx/geometry/rect_conversions.h"

#include "accessibility_bridge.h"

namespace flutter {

FlutterPlatformNodeDelegate::FlutterPlatformNodeDelegate() = default;

FlutterPlatformNodeDelegate::~FlutterPlatformNodeDelegate() = default;

void FlutterPlatformNodeDelegate::Init(AccessibilityBridge* bridge, ui::AXNode* node) {
  bridge_ = bridge;
  ax_node_ = node;
}

AccessibilityBridge* FlutterPlatformNodeDelegate::GetBridge() const {
  return bridge_;
}

ui::AXNode* FlutterPlatformNodeDelegate::GetAXNode() const {
  return ax_node_;
}

bool FlutterPlatformNodeDelegate::AccessibilityPerformAction(
    const ui::AXActionData& data) {
  ui::AXNode::AXID target = GetAXNode()->id();
  switch (data.action) {
    case ax::mojom::Action::kDoDefault:
      bridge_->DispatchAccessibilityAction(
          target, FlutterSemanticsAction::kFlutterSemanticsActionTap, {nullptr},
          0);
      return true;
    case ax::mojom::Action::kFocus:
      bridge_->SetFocusedNode(target);
      bridge_->DispatchAccessibilityAction(
          target,
          FlutterSemanticsAction::
              kFlutterSemanticsActionDidGainAccessibilityFocus,
          {nullptr}, 0);
      return true;
    case ax::mojom::Action::kScrollToMakeVisible:
      bridge_->DispatchAccessibilityAction(
          target, FlutterSemanticsAction::kFlutterSemanticsActionShowOnScreen,
          {nullptr}, 0);
      return true;
    // TODO(chunhtai): support more actions.
    default:
      return false;
  }
  return false;
}

const ui::AXNodeData& FlutterPlatformNodeDelegate::GetData() const {
  return GetAXNode()->data();
}

gfx::NativeViewAccessible FlutterPlatformNodeDelegate::GetParent() {
  if (!GetAXNode()->parent()) {
    return nullptr;
  }
  std::shared_ptr<FlutterPlatformNodeDelegate> accessibility =
      bridge_->GetFlutterPlatformNodeDelegateFromID(GetAXNode()->parent()->id())
          .lock();
  BASE_CHECK(accessibility);
  return accessibility->GetNativeViewAccessible();
}

gfx::NativeViewAccessible FlutterPlatformNodeDelegate::GetFocus() {
  ui::AXNode::AXID focused_node = bridge_->GetLastFocusedNode();
  if (focused_node == ui::AXNode::kInvalidAXID) {
    return nullptr;
  }
  std::weak_ptr<FlutterPlatformNodeDelegate> focus =
      bridge_->GetFlutterPlatformNodeDelegateFromID(focused_node);
  auto focus_ptr = focus.lock();
  if (!focus_ptr)
    return nullptr;
  return focus_ptr->GetNativeViewAccessible();
}

int FlutterPlatformNodeDelegate::GetChildCount() const {
  return static_cast<int>(GetAXNode()->GetUnignoredChildCount());
}

gfx::NativeViewAccessible FlutterPlatformNodeDelegate::ChildAtIndex(int index) {
  ui::AXNode::AXID child = GetAXNode()->GetUnignoredChildAtIndex(index)->id();
  return bridge_->GetFlutterPlatformNodeDelegateFromID(child)
      .lock()
      ->GetNativeViewAccessible();
}

gfx::Rect FlutterPlatformNodeDelegate::GetBoundsRect(
    const ui::AXCoordinateSystem coordinate_system,
    const ui::AXClippingBehavior clipping_behavior,
    ui::AXOffscreenResult* offscreen_result) const {
  // TODO(chunhtai): consider screen dpr.
  const bool clip_bounds =
      clipping_behavior == ui::AXClippingBehavior::kClipped;
  bool offscreen = false;
  gfx::RectF bounds =
      bridge_->RelativeToGlobalBounds(GetAXNode(), &offscreen, clip_bounds);
  if (offscreen_result != nullptr) {
    *offscreen_result = offscreen ? ui::AXOffscreenResult::kOffscreen
                                  : ui::AXOffscreenResult::kOnscreen;
  }
  return gfx::ToEnclosingRect(bounds);
}

}  // namespace flutter
