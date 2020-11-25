// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_accessibility.h"

#include "accessibility_bridge.h"
#include "ax/ax_action_data.h"

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
      DispatchAccessibilityAction(target, FlutterSemanticsAction::kFlutterSemanticsActionTap, nullptr, 0);
      return true;
    case ax::Action::kFocus:
      bridge_->SetFocusedNode(target);
      DispatchAccessibilityAction(target, FlutterSemanticsAction::kFlutterSemanticsActionDidGainAccessibilityFocus, nullptr, 0);
      return true;
    // case ax::Action::kScrollToPoint: {
    //   // Convert the target point from screen coordinates to frame coordinates.
    //   gfx::Point target =
    //       data.target_point - manager_->GetRoot()
    //                               ->GetUnclippedScreenBoundsRect()
    //                               .OffsetFromOrigin();
    //   manager_->ScrollToPoint(*this, target);
    //   return true;
    // }
    case ax::Action::kScrollToMakeVisible:
      DispatchAccessibilityAction(target, FlutterSemanticsAction::kFlutterSemanticsActionShowOnScreen, nullptr, 0);
      return true;
    // case ax::Action::kSetScrollOffset:
    //   manager_->SetScrollOffset(*this, data.target_point);
    //   return true;
    // case ax::Action::kSetSelection: {
    //   // "data.anchor_offset" and "data.focus_ofset" might need to be adjusted
    //   // if the anchor or the focus nodes include ignored children.
    //   ui::AXActionData selection = data;
    //   const BrowserAccessibility* anchor_object =
    //       manager()->GetFromID(selection.anchor_node_id);
    //   DCHECK(anchor_object);
    //   if (!anchor_object->PlatformIsLeaf()) {
    //     DCHECK_GE(selection.anchor_offset, 0);
    //     const BrowserAccessibility* anchor_child =
    //         anchor_object->InternalGetChild(uint32_t{selection.anchor_offset});
    //     if (anchor_child) {
    //       selection.anchor_offset =
    //           int{anchor_child->node()->index_in_parent()};
    //       selection.anchor_node_id = anchor_child->node()->parent()->id();
    //     } else {
    //       // Since the child was not found, the only alternative is that this is
    //       // an "after children" position.
    //       selection.anchor_offset =
    //           int{anchor_object->node()->children().size()};
    //     }
    //   }

    //   const BrowserAccessibility* focus_object =
    //       manager()->GetFromID(selection.focus_node_id);
    //   DCHECK(focus_object);
    //   if (!focus_object->PlatformIsLeaf()) {
    //     DCHECK_GE(selection.focus_offset, 0);
    //     const BrowserAccessibility* focus_child =
    //         focus_object->InternalGetChild(uint32_t{selection.focus_offset});
    //     if (focus_child) {
    //       selection.focus_offset = int{focus_child->node()->index_in_parent()};
    //       selection.focus_node_id = focus_child->node()->parent()->id();
    //     } else {
    //       // Since the child was not found, the only alternative is that this is
    //       // an "after children" position.
    //       selection.focus_offset = int{focus_object->node()->children().size()};
    //     }
    //   }

    //   manager_->SetSelection(selection);
    //   return true;
    // }
    // case ax::Action::kSetValue:
    //   manager_->SetValue(*this, data.value);
    //   return true;
    // case ax::Action::kSetSequentialFocusNavigationStartingPoint:
    //   manager_->SetSequentialFocusNavigationStartingPoint(*this);
    //   return true;
    // case ax::Action::kShowContextMenu:
    //   manager_->ShowContextMenu(*this);
    //   return true;
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
  return GetBridge()->GetFlutterAccessibilityFromID(GetAXNode()->parent()->id())->GetNativeViewAccessible();
}

gfx::NativeViewAccessible FlutterAccessibility::GetFocus() {
  int32_t focused_node = GetBridge()->GetLastFocusedNode();
  if (focused_node == ax::AXNode::kInvalidAXID) {
    return nullptr;
  }
  FlutterAccessibility* focus = GetBridge()->GetFlutterAccessibilityFromID(focused_node);
  if (!focus)
    return nullptr;
  return GetBridge()->GetFlutterAccessibilityFromID(focused_node)->GetNativeViewAccessible();
}

int FlutterAccessibility::GetChildCount() const {
  return static_cast<int>(GetAXNode()->GetUnignoredChildCount());
}

gfx::NativeViewAccessible FlutterAccessibility::ChildAtIndex(int index) {
  int32_t child = GetAXNode()->GetUnignoredChildAtIndex(index)->id();
  return GetBridge()->GetFlutterAccessibilityFromID(child)->GetNativeViewAccessible();
}

}  // namespace ax
