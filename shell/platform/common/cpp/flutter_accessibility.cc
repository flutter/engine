// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_accessibility.h"

#include "flutter/third_party/accessibility/ax/ax_action_data.h"
#include "flutter/third_party/accessibility/gfx/geometry/rect_conversions.h"

#include "accessibility_bridge.h"

namespace ui {

FlutterAccessibility::FlutterAccessibility() = default;

FlutterAccessibility::~FlutterAccessibility() = default;

void FlutterAccessibility::Init(AccessibilityBridge* bridge, AXNode* node) {
  bridge_ = bridge;
  ax_node_ = node;
}

AccessibilityBridge* FlutterAccessibility::GetBridge() const {
  return bridge_;
}

AXNode* FlutterAccessibility::GetAXNode() const {
  return ax_node_;
}

bool FlutterAccessibility::AccessibilityPerformAction(
    const AXActionData& data) {
  int32_t target = GetAXNode()->id();
  switch (data.action) {
    case ax::mojom::Action::kDoDefault:
      bridge_->GetDelegate()->DispatchAccessibilityAction(
          target, FlutterSemanticsAction::kFlutterSemanticsActionTap, nullptr,
          0);
      return true;
    case ax::mojom::Action::kFocus:
      bridge_->SetFocusedNode(target);
      bridge_->GetDelegate()->DispatchAccessibilityAction(
          target,
          FlutterSemanticsAction::
              kFlutterSemanticsActionDidGainAccessibilityFocus,
          nullptr, 0);
      return true;
    case ax::mojom::Action::kScrollToMakeVisible:
      bridge_->GetDelegate()->DispatchAccessibilityAction(
          target, FlutterSemanticsAction::kFlutterSemanticsActionShowOnScreen,
          nullptr, 0);
      return true;
    // TODO(chunhtai): support more actions.
    default:
      return false;
  }
  return false;
}

const AXNodeData& FlutterAccessibility::GetData() const {
  return GetAXNode()->data();
}

gfx::NativeViewAccessible FlutterAccessibility::GetParent() {
  if (!GetAXNode()->parent()) {
    return nullptr;
  }
  return GetBridge()
      ->GetFlutterAccessibilityFromID(GetAXNode()->parent()->id())
      ->GetNativeViewAccessible();
}

gfx::NativeViewAccessible FlutterAccessibility::GetFocus() {
  int32_t focused_node = GetBridge()->GetLastFocusedNode();
  if (focused_node == AXNode::kInvalidAXID) {
    return nullptr;
  }
  FlutterAccessibility* focus =
      GetBridge()->GetFlutterAccessibilityFromID(focused_node);
  if (!focus)
    return nullptr;
  return GetBridge()
      ->GetFlutterAccessibilityFromID(focused_node)
      ->GetNativeViewAccessible();
}

int FlutterAccessibility::GetChildCount() const {
  return static_cast<int>(GetAXNode()->GetUnignoredChildCount());
}

gfx::NativeViewAccessible FlutterAccessibility::ChildAtIndex(int index) {
  int32_t child = GetAXNode()->GetUnignoredChildAtIndex(index)->id();
  return GetBridge()
      ->GetFlutterAccessibilityFromID(child)
      ->GetNativeViewAccessible();
}

gfx::Rect FlutterAccessibility::GetBoundsRect(
    const AXCoordinateSystem coordinate_system,
    const AXClippingBehavior clipping_behavior,
    AXOffscreenResult* offscreen_result) const {
  // TODO(chunhtai): consider screen dpr.
  const bool clip_bounds = clipping_behavior == AXClippingBehavior::kClipped;
  bool offscreen = false;
  gfx::RectF bounds = GetBridge()->GetAXTree()->RelativeToTreeBounds(
      GetAXNode(), gfx::RectF(), &offscreen, clip_bounds);
  *offscreen_result =
      offscreen ? AXOffscreenResult::kOffscreen : AXOffscreenResult::kOnscreen;
  return gfx::ToEnclosingRect(bounds);
}

}  // namespace ui
