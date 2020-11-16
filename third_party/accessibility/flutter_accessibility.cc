// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_accessibility.h"



namespace ax {

FlutterAccessibility::FlutterAccessibility() = default;

FlutterAccessibility::~FlutterAccessibility() = default;

AccessibilityBridge* FlutterAccessibility::GetBridge() {
  return bridge_;
}

bool FlutterAccessibility::AccessibilityPerformAction(
    const ax::AXActionData& data) {
  int32_t target = data.target_node_id;
  FML_LOG(ERROR) << "got action " <<ax::ToString(data.action);
  switch (data.action) {
     case ax::Action::kDoDefault:
      action = FlutterSemanticsAction::kFlutterSemanticsActionTap;
      DispatchAccessibilityAction(target, FlutterSemanticsAction::kFlutterSemanticsActionTap, null, 0);
      return true;
    case ax::Action::kFocus:
      // Notifies previous node has loose focus
      bridge_->SetFocusedNode(target);
      DispatchAccessibilityAction(target, FlutterSemanticsAction::kFlutterSemanticsActionDidGainAccessibilityFocus, null, 0);
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
    // case ax::Action::kScrollToMakeVisible:
    //   manager_->ScrollToMakeVisible(
    //       *this, data.target_rect, data.horizontal_scroll_alignment,
    //       data.vertical_scroll_alignment, data.scroll_behavior);
    //   return true;
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

void FlutterAccessibility::Init(AccessibilityBridge* bridge, AXNode* node) {
  bridge_ = bridge;
}

void FlutterAccessibility::OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event) {}
}  // namespace ax
