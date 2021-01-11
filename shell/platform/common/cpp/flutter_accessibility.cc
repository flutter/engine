// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_accessibility.h"

#include "flutter/third_party/accessibility/ax/ax_action_data.h"
#include "flutter/third_party/accessibility/gfx/geometry/rect_conversions.h"

#include "accessibility_bridge.h"

namespace flutter {

FlutterAccessibility::FlutterAccessibility() = default;

FlutterAccessibility::~FlutterAccessibility() = default;

void FlutterAccessibility::Init(AccessibilityBridge* bridge, ui::AXNode* node) {
  bridge_ = bridge;
  ax_node_ = node;
}

AccessibilityBridge* FlutterAccessibility::GetBridge() const {
  return bridge_;
}

ui::AXNode* FlutterAccessibility::GetAXNode() const {
  return ax_node_;
}

bool FlutterAccessibility::AccessibilityPerformAction(
    const ui::AXActionData& data) {
  int32_t target = GetAXNode()->id();
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

const ui::AXNodeData& FlutterAccessibility::GetData() const {
  return GetAXNode()->data();
}

gfx::NativeViewAccessible FlutterAccessibility::GetParent() {
  if (!GetAXNode()->parent()) {
    return nullptr;
  }
  std::shared_ptr<FlutterAccessibility> accessibility =
      bridge_->GetFlutterAccessibilityFromID(GetAXNode()->parent()->id())
          .lock();
  BASE_CHECK(accessibility);
  return accessibility->GetNativeViewAccessible();
}

gfx::NativeViewAccessible FlutterAccessibility::GetFocus() {
  int32_t focused_node = bridge_->GetLastFocusedNode();
  if (focused_node == ui::AXNode::kInvalidAXID) {
    return nullptr;
  }
  std::weak_ptr<FlutterAccessibility> focus =
      bridge_->GetFlutterAccessibilityFromID(focused_node);
  auto focus_ptr = focus.lock();
  if (!focus_ptr)
    return nullptr;
  return focus_ptr->GetNativeViewAccessible();
}

int FlutterAccessibility::GetChildCount() const {
  return static_cast<int>(GetAXNode()->GetUnignoredChildCount());
}

gfx::NativeViewAccessible FlutterAccessibility::ChildAtIndex(int index) {
  int32_t child = GetAXNode()->GetUnignoredChildAtIndex(index)->id();
  return bridge_->GetFlutterAccessibilityFromID(child)
      .lock()
      ->GetNativeViewAccessible();
}

gfx::Rect FlutterAccessibility::GetBoundsRect(
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
