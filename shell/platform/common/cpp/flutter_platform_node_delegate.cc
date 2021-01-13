// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_platform_node_delegate.h"

#include "flutter/third_party/accessibility/ax/ax_action_data.h"
#include "flutter/third_party/accessibility/gfx/geometry/rect_conversions.h"

namespace flutter {

FlutterPlatformNodeDelegate::FlutterPlatformNodeDelegate() = default;

FlutterPlatformNodeDelegate::~FlutterPlatformNodeDelegate() = default;

void FlutterPlatformNodeDelegate::Init(OwnerBridge* bridge, ui::AXNode* node) {
  bridge_ = bridge;
  ax_node_ = node;
}

ui::AXNode* FlutterPlatformNodeDelegate::GetAXNode() const {
  return ax_node_;
}

bool FlutterPlatformNodeDelegate::AccessibilityPerformAction(
    const ui::AXActionData& data) {
  AccessibilityNodeId target = ax_node_->id();
  switch (data.action) {
    case ax::mojom::Action::kDoDefault:
      bridge_->DispatchAccessibilityAction(
          target, FlutterSemanticsAction::kFlutterSemanticsActionTap, {});
      return true;
    case ax::mojom::Action::kFocus:
      bridge_->SetLastFocusedId(target);
      bridge_->DispatchAccessibilityAction(
          target,
          FlutterSemanticsAction::
              kFlutterSemanticsActionDidGainAccessibilityFocus,
          {});
      return true;
    case ax::mojom::Action::kScrollToMakeVisible:
      bridge_->DispatchAccessibilityAction(
          target, FlutterSemanticsAction::kFlutterSemanticsActionShowOnScreen,
          {});
      return true;
    // TODO(chunhtai): support more actions.
    default:
      return false;
  }
  return false;
}

const ui::AXNodeData& FlutterPlatformNodeDelegate::GetData() const {
  return ax_node_->data();
}

gfx::NativeViewAccessible FlutterPlatformNodeDelegate::GetParent() {
  if (!ax_node_->parent()) {
    return nullptr;
  }
  return bridge_->GetNativeAccessibleFromId(ax_node_->parent()->id());
}

gfx::NativeViewAccessible FlutterPlatformNodeDelegate::GetFocus() {
  AccessibilityNodeId last_focused = bridge_->GetLastFocusedId();
  if (last_focused == ui::AXNode::kInvalidAXID) {
    return nullptr;
  }
  return bridge_->GetNativeAccessibleFromId(last_focused);
}

int FlutterPlatformNodeDelegate::GetChildCount() const {
  return static_cast<int>(ax_node_->GetUnignoredChildCount());
}

gfx::NativeViewAccessible FlutterPlatformNodeDelegate::ChildAtIndex(int index) {
  AccessibilityNodeId child = ax_node_->GetUnignoredChildAtIndex(index)->id();
  return bridge_->GetNativeAccessibleFromId(child);
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
      bridge_->RelativeToGlobalBounds(ax_node_, offscreen, clip_bounds);
  if (offscreen_result != nullptr) {
    *offscreen_result = offscreen ? ui::AXOffscreenResult::kOffscreen
                                  : ui::AXOffscreenResult::kOnscreen;
  }
  return gfx::ToEnclosingRect(bounds);
}

}  // namespace flutter
