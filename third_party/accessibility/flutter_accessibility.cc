// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_accessibility.h"

#include "ax/ax_action_data.h"

#include "accessibility_bridge.h"

namespace ax {

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
    const ax::AXActionData& data) {
  int32_t target = GetAXNode()->id();
  switch (data.action) {
    case ax::Action::kDoDefault:
      DispatchAccessibilityAction(
          target, FlutterSemanticsAction::kFlutterSemanticsActionTap, nullptr,
          0);
      return true;
    case ax::Action::kFocus:
      bridge_->SetFocusedNode(target);
      DispatchAccessibilityAction(
          target,
          FlutterSemanticsAction::
              kFlutterSemanticsActionDidGainAccessibilityFocus,
          nullptr, 0);
      return true;
    case ax::Action::kScrollToMakeVisible:
      DispatchAccessibilityAction(
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
  if (focused_node == ax::AXNode::kInvalidAXID) {
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

}  // namespace ax
