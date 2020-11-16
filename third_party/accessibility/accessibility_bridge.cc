// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "accessibility_bridge.h"

#include <functional>
#include <utility>

#include "flutter/fml/logging.h"

#include "ax/ax_tree_update.h"

namespace ax { // namespace

// AccessibilityBridge
AccessibilityBridge::AccessibilityBridge(void* user_data)
  : tree_(std::make_unique<AXTree>()),
    event_generator_(tree_.get()) {
  user_data_ = user_data;
  tree_->AddObserver((ax::AXTreeObserver *)this);
}

AccessibilityBridge::~AccessibilityBridge() {
  event_generator_.ReleaseTree();
  tree_->RemoveObserver((ax::AXTreeObserver *)this);
}

void AccessibilityBridge::AddFlutterSemanticsNodeUpdate(const FlutterSemanticsNode* node) {
  _pending_semantics_node_updates[node->id] = FromFlutterSemanticsNode(node);
}

void AccessibilityBridge::AddFlutterSemanticsCustomActionUpdate(const FlutterSemanticsCustomAction* action) {
  _pending_semantics_custom_action_updates[action->id] = FromFlutterSemanticsCustomAction(action);
}

void AccessibilityBridge::CommitUpdates() {
  AXTreeUpdate update;
  // Figure out update order, AXTree only accepts update in tree order, where
  // parent node must come before the child node in AXTreeUpdate.nodes.
  // We start with picking a random node and turn the entire subtree into a
  // list. We pick another node from the remaining update, and keep doing so until
  // the update map is empty.
  // We then concatenate the lists in the reversed order, this guarantees parent
  // updates always come before child updates.
  std::vector<std::vector<SemanticsNode>> results;
  while (!_pending_semantics_node_updates.empty()) {
    auto begin = _pending_semantics_node_updates.begin();
    SemanticsNode target = begin->second;
    std::vector<SemanticsNode> sub_tree_list;
    GetSubTreeList(target, sub_tree_list);
    results.push_back(sub_tree_list);
    _pending_semantics_node_updates.erase(begin);
  }

  for (size_t i = results.size(); i > 0; i--) {
    for (SemanticsNode node : results[i - 1]) {
      AXNodeData node_data;
      ConvertFluterUpdateToAXNodeData(node, node_data);
      update.nodes.push_back(node_data);
    }
  }

  tree_->Unserialize(update);
  _pending_semantics_node_updates.clear();
  _pending_semantics_custom_action_updates.clear();

  std::string error = tree_->error();
  if (!error.empty()) {
    FML_LOG(ERROR) << "Failed to update AXTree, error: " << error;
    return;
  }
  
  
  for (const auto& targeted_event : event_generator_) {
    FlutterAccessibility* event_target = GetFlutterAccessibilityFromID(targeted_event.node->id());
    if (!event_target)
      continue;

    event_target->OnAccessibilityEvent(targeted_event);
  }
  event_generator_.ClearEvents();
}

void* AccessibilityBridge::GetUserData() {
  return user_data_;
}

ax::AXTree* AccessibilityBridge::GetAXTree() {
  return tree_.get();
}

AXEventGenerator* AccessibilityBridge::GetEventGenerator() {
  return &event_generator_;
}

FlutterAccessibility* AccessibilityBridge::GetFlutterAccessibilityFromID(int32_t id) const {
  const auto iter = id_wrapper_map_.find(id);
  if (iter != id_wrapper_map_.end())
    return iter->second;

  return nullptr;
}

void AccessibilityBridge::SetFocusedNode(int32_t node_id) {
  if (last_focused_node_ != node_id) {
    FlutterAccessibility* last_focused_child = GetFlutterAccessibilityFromID(last_focused_node_);
    if (last_focused_child) {
      last_focused_child->DispatchAccessibilityAction(last_focused_node_, FlutterSemanticsAction::kFlutterSemanticsActionDidLoseAccessibilityFocus, null, 0);
    }
    last_focused_node_ = node_id;
  }
}

void AccessibilityBridge::OnNodeWillBeDeleted(ax::AXTree* tree, ax::AXNode* node) {}

void AccessibilityBridge::OnSubtreeWillBeDeleted(ax::AXTree* tree,
                                                         ax::AXNode* node) {}

void AccessibilityBridge::OnNodeCreated(ax::AXTree* tree,
                                                ax::AXNode* node) {
  FML_DCHECK(node);
  FlutterAccessibility* wrapper = FlutterAccessibility::Create();
  id_wrapper_map_[node->id()] = wrapper;
  wrapper->Init(this, node);
}

void AccessibilityBridge::OnNodeDeleted(ax::AXTree* tree,
                                                int32_t node_id) {
  FML_DCHECK(node_id != ax::AXNode::kInvalidAXID);
  if (FlutterAccessibility* wrapper = GetFlutterAccessibilityFromID(node_id)) {
    id_wrapper_map_.erase(node_id);
    delete wrapper;
  }
}

void AccessibilityBridge::OnNodeReparented(ax::AXTree* tree,
                                                   ax::AXNode* node) {

}

void AccessibilityBridge::OnRoleChanged(ax::AXTree* tree,
                                                ax::AXNode* node,
                                                ax::Role old_role,
                                                ax::Role new_role) {

}

void AccessibilityBridge::OnAtomicUpdateFinished(
    ax::AXTree* tree,
    bool root_changed,
    const std::vector<ax::AXTreeObserver::Change>& changes) {
  // The Flutter semantics update does not include child->parent relationship
  // We have to update the relative bound offset container id here in order
  // to calculate the screen bound correctly.
  for (const auto& change : changes) {
    ax::AXNode* node = change.node;
    const AXNodeData& data = node->data();
    int32_t offset_container_id = -1;
    if (node->parent()) {
      offset_container_id = node->parent()->id();
    }
    node->SetLocation(offset_container_id, data.relative_bounds.bounds, data.relative_bounds.transform);
  }
}

void AccessibilityBridge::GetSubTreeList(SemanticsNode target, std::vector<SemanticsNode>& result) {
  result.push_back(target);
  for (int32_t child : target.children_in_traversal_order) {
    auto iter = _pending_semantics_node_updates.find(child);
    if (iter != _pending_semantics_node_updates.end()) {
      SemanticsNode node = iter->second;
      GetSubTreeList(node, result);
      _pending_semantics_node_updates.erase(iter);
    }
  }
}

void AccessibilityBridge::ConvertFluterUpdateToAXNodeData(const SemanticsNode& node, AXNodeData& node_data) {
  node_data.id = node.id;
  SetRoleFromFlutterUpdate(node_data, node);
  SetStateFromFlutterUpdate(node_data, node);
  SetActionsFromFlutterUpdate(node_data, node);
  SetBooleanAttributesFromFlutterUpdate(node_data, node);
  SetIntAttributesFromFlutterUpdate(node_data, node);
  SetIntListAttributesFromFlutterUpdate(node_data, node);
  SetStringListAttributesFromFlutterUpdate(node_data, node);
  SetNameFromFlutterUpdate(node_data, node);
  node_data.SetValue(node.value);
  node_data.relative_bounds.bounds.setLTRB(
    node.rect.left,
    node.rect.top,
    node.rect.right,
    node.rect.bottom
  );
  node_data.relative_bounds.transform.setAll(
    node.transform.scaleX,
    node.transform.skewX,
    node.transform.transX,
    node.transform.skewY,
    node.transform.scaleY,
    node.transform.transY,
    node.transform.pers0,
    node.transform.pers1,
    node.transform.pers2
  );
  for (auto child: node.children_in_traversal_order) {
    node_data.child_ids.push_back(child);
  }
}

void AccessibilityBridge::SetRoleFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node) {
  FlutterSemanticsFlag flags = node.flags;
  if (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsButton) {
    node_data.role = Role::kButton;
    return;
  }
  if (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsTextField) {
    node_data.role = Role::kTextField;
    return;
  }
  if (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsHeader) {
    node_data.role = Role::kHeader;
    return;
  }
  if (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsImage) {
    node_data.role = Role::kImage;
    return;
  }
  if (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsLink) {
    node_data.role = Role::kLink;
    return;
  }

  if (flags & kFlutterSemanticsFlagIsInMutuallyExclusiveGroup &&
      flags & kFlutterSemanticsFlagHasCheckedState) {
    node_data.role = Role::kRadioButton;
    return;
  }
  if (flags & kFlutterSemanticsFlagHasCheckedState) {
    node_data.role = Role::kCheckBox;
    return;
  }
  // If the state cannot be determined by flutter flags, we fallback to group
  // or static text.
  node_data.role = node.children_in_traversal_order.size() == 0 ? Role::kStaticText : Role::kGroup;
}

void AccessibilityBridge::SetStateFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node) {
  FlutterSemanticsFlag flags = node.flags;
  if (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsTextField && 
      (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsReadOnly) > 0) {
    node_data.AddState(State::kEditable);
  }
  if (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsHidden ) {
      // flags & FlutterSemanticsFlag::kFlutterSemanticsFlagScopesRoute ||
      // ((actions & khasScrollingAction) == 0 && flags == 0 && node.value.empty() && node.label.empty() && node.hint.empty())) {
    node_data.AddState(State::kInvisible);
  }
  // if (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsFocusable) {
  //   node_data.AddState(State::kFocusable);
  // }
  node_data.AddState(State::kFocusable);
}

void AccessibilityBridge::SetActionsFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node) {
  FlutterSemanticsAction actions = node.actions;
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionTap) {
    node_data.AddAction(Action::kDoDefault);
  }
  // if (actions & FlutterSemanticsAction::kFlutterSemanticsActionLongPress) {
    
  // }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionScrollLeft) {
    node_data.AddAction(Action::kScrollLeft);
  }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionScrollRight) {
    node_data.AddAction(Action::kScrollRight);
  }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionScrollUp) {
    node_data.AddAction(Action::kScrollUp);
  }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionScrollDown) {
    node_data.AddAction(Action::kScrollDown);
  }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionIncrease) {
    node_data.AddAction(Action::kIncrement);
  }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionDecrease) {
    node_data.AddAction(Action::kDecrement);
  }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionShowOnScreen) {
    node_data.AddAction(Action::kScrollToMakeVisible);
  }
  // if (actions & FlutterSemanticsAction::kFlutterSemanticsActionMoveCursorForwardByCharacter) {
    
  // }
  // if (actions & FlutterSemanticsAction::kFlutterSemanticsActionMoveCursorBackwardByCharacter) {
    
  // }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionSetSelection) {
    node_data.AddAction(Action::kSetSelection);
  }
  // if (actions & FlutterSemanticsAction::kFlutterSemanticsActionCopy) {
    
  // }
  // if (actions & FlutterSemanticsAction::kFlutterSemanticsActionCut) {
    
  // }
  // if (actions & FlutterSemanticsAction::kFlutterSemanticsActionPaste) {
    
  // }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionDidGainAccessibilityFocus) {
    node_data.AddAction(Action::kSetAccessibilityFocus);
  }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionDidLoseAccessibilityFocus) {
    node_data.AddAction(Action::kClearAccessibilityFocus);
  }
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionCustomAction) {
    node_data.AddAction(Action::kCustomAction);
  }
  // if (actions & FlutterSemanticsAction::kFlutterSemanticsActionDismiss) {

  // }
  // if (actions & FlutterSemanticsAction::kFlutterSemanticsActionMoveCursorForwardByWord) {

  // }
  // if (actions & FlutterSemanticsAction::kFlutterSemanticsActionMoveCursorBackwardByWord) {

  // }
}

void AccessibilityBridge::SetBooleanAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node) {
  // FlutterSemanticsAction actions = node.actions;
  // FlutterSemanticsFlag flags = node.flags;
  // node_data.AddBoolAttribute(BoolAttribute::kScrollable, actions & khasScrollingAction);
  // node_data.AddBoolAttribute(BoolAttribute::kClickable, actions & FlutterSemanticsAction::kFlutterSemanticsActionTap);
  // node_data.AddBoolAttribute(BoolAttribute::kClipsChildren, actions & khasScrollingAction && node.children_in_traversal_order.size() != 0);
  // node_data.AddBoolAttribute(BoolAttribute::kSelected, flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsSelected);
  // node_data.AddBoolAttribute(BoolAttribute::kEditableRoot, flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsTextField && 
  //                                                                     (flags & FlutterSemanticsFlag::kFlutterSemanticsFlagIsReadOnly) > 0);
}

void AccessibilityBridge::SetIntAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node) {
  node_data.AddIntAttribute(IntAttribute::kTextDirection, node.text_direction);
}

void AccessibilityBridge::SetIntListAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node) {
  FlutterSemanticsAction actions = node.actions;
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionCustomAction) {
    std::vector<int32_t> custom_action_ids;
    for (size_t i = 0; i < node.custom_accessibility_actions.size(); i++) {
      custom_action_ids.push_back(node.custom_accessibility_actions[i]);
    }
    node_data.AddIntListAttribute(IntListAttribute::kCustomActionIds, custom_action_ids);
  }
}

void AccessibilityBridge::SetStringListAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node) {
  FlutterSemanticsAction actions = node.actions;
  if (actions & FlutterSemanticsAction::kFlutterSemanticsActionCustomAction) {
    std::vector<std::string> custom_action_description;
    for (size_t i = 0; i < node.custom_accessibility_actions.size(); i++) {
      auto iter = _pending_semantics_custom_action_updates.find(node.custom_accessibility_actions[i]);
      FML_DCHECK(iter != _pending_semantics_custom_action_updates.end());
      custom_action_description.push_back(iter->second.label);
    }
    node_data.AddStringListAttribute(StringListAttribute::kCustomActionDescriptions, custom_action_description);
  }
}

void AccessibilityBridge::SetNameFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node) {
  std::string name = node.label;
  node_data.SetName(name);
}

AccessibilityBridge::SemanticsNode AccessibilityBridge::FromFlutterSemanticsNode(const FlutterSemanticsNode* flutter_node) {
  SemanticsNode result;
  result.id = flutter_node->id;
  result.flags = flutter_node->flags;
  result.actions = flutter_node->actions;
  result.text_selection_base = flutter_node->text_selection_base;
  result.text_selection_extent = flutter_node->text_selection_extent;
  result.scroll_child_count = flutter_node->scroll_child_count;
  result.scroll_index = flutter_node->scroll_index;
  result.scroll_position = flutter_node->scroll_position;
  result.scroll_extent_max = flutter_node->scroll_extent_max;
  result.scroll_extent_min = flutter_node->scroll_extent_min;
  result.elevation = flutter_node->elevation;
  result.thickness = flutter_node->thickness;
  if (flutter_node->label) {
    result.label = std::string(flutter_node->label);
  }
  if (flutter_node->hint) {
    result.hint = std::string(flutter_node->hint);
  }
  if (flutter_node->value) {
    result.value = std::string(flutter_node->value);
  }
  result.increased_value = std::string(flutter_node->increased_value);
  result.decreased_value = std::string(flutter_node->decreased_value);
  result.text_direction = flutter_node->text_direction;
  result.rect = flutter_node->rect;
  result.transform = flutter_node->transform;
  result.children_in_traversal_order = std::vector<int32_t>(flutter_node->children_in_traversal_order, flutter_node->children_in_traversal_order + flutter_node->child_count);
  result.custom_accessibility_actions = std::vector<int32_t>(flutter_node->custom_accessibility_actions, flutter_node->custom_accessibility_actions + flutter_node->custom_accessibility_actions_count);
  return result;
}

AccessibilityBridge::SemanticsCustomAction AccessibilityBridge::FromFlutterSemanticsCustomAction(const FlutterSemanticsCustomAction* flutter_custom_action) {
  SemanticsCustomAction result;
  result.id = flutter_custom_action->id;
  result.override_action = flutter_custom_action->override_action;
  if (flutter_custom_action->label) {
    result.label = std::string(flutter_custom_action->label);
  }
  if (flutter_custom_action->hint) {
    result.hint = std::string(flutter_custom_action->hint);
  }
  return result;
}

}  // namespace ax
