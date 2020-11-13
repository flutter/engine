// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_ACCESSIBILITYBRIDGE_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_ACCESSIBILITYBRIDGE_H_

#include <unordered_map>

#include "flutter/shell/platform/embedder/embedder.h"

#include "ax/ax_event_generator.h"
#include "ax/ax_tree_observer.h"
#include "ax/ax_tree.h"
#include "ax/platform/ax_platform_node_delegate.h"

#include "flutter_accessibility.h"

namespace ax {

class AccessibilityBridge : public AXTreeObserver {
 public:
  AccessibilityBridge(void* user_data);
  ~AccessibilityBridge();

  void AddFlutterSemanticsNodeUpdate(const FlutterSemanticsNode* node);
  void AddFlutterSemanticsCustomActionUpdate(const FlutterSemanticsCustomAction* action);
  void CommitUpdates();
  ax::AXTree* GetAXTree();
  AXEventGenerator* GetEventGenerator();
  void* GetUserData();
  FlutterAccessibility* GetFlutterAccessibilityFromID(int32_t id) const;
  // AXTreeObserver implementation.
  void OnNodeWillBeDeleted(ax::AXTree* tree, ax::AXNode* node) override;
  void OnSubtreeWillBeDeleted(ax::AXTree* tree, ax::AXNode* node) override;
  void OnNodeCreated(ax::AXTree* tree, ax::AXNode* node) override;
  void OnNodeDeleted(ax::AXTree* tree, int32_t node_id) override;
  void OnNodeReparented(ax::AXTree* tree, ax::AXNode* node) override;
  void OnRoleChanged(ax::AXTree* tree,
                     ax::AXNode* node,
                     ax::Role old_role,
                     ax::Role new_role) override;
  void OnAtomicUpdateFinished(
      ax::AXTree* tree,
      bool root_changed,
      const std::vector<ax::AXTreeObserver::Change>& changes) override;



 private:
  // See FlutterSemanticsNode in embedder.h
  typedef struct {
    int32_t id;
    FlutterSemanticsFlag flags;
    FlutterSemanticsAction actions;
    int32_t text_selection_base;
    int32_t text_selection_extent;
    int32_t scroll_child_count;
    int32_t scroll_index;
    double scroll_position;
    double scroll_extent_max;
    double scroll_extent_min;
    double elevation;
    double thickness;
    std::string label;
    std::string hint;
    std::string value;
    std::string increased_value;
    std::string decreased_value;
    FlutterTextDirection text_direction;
    FlutterRect rect;
    FlutterTransformation transform;
    std::vector<int32_t> children_in_traversal_order;
    std::vector<int32_t> custom_accessibility_actions;
  } SemanticsNode;

  // See FlutterSemanticsCustomAction in embedder.h
  typedef struct {
    int32_t id;
    FlutterSemanticsAction override_action;
    std::string label;
    std::string hint;
  } SemanticsCustomAction;

  std::unordered_map<int32_t, FlutterAccessibility*> id_wrapper_map_;
  std::unique_ptr<AXTree> tree_;
  AXEventGenerator event_generator_;
  std::unordered_map<int32_t, SemanticsNode> _pending_semantics_node_updates;
  std::unordered_map<int32_t, SemanticsCustomAction> _pending_semantics_custom_action_updates;
  void* user_data_;

  void InitAXTree(const AXTreeUpdate& initial_state);
  void GetSubTreeList(SemanticsNode target, std::vector<SemanticsNode>& result);
  void ConvertFluterUpdateToAXNodeData(const SemanticsNode& node, AXNodeData& node_data);
  void SetRoleFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetStateFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetActionsFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetBooleanAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetIntAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetIntListAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetStringListAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetNameFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  SemanticsNode FromFlutterSemanticsNode(const FlutterSemanticsNode* flutter_node);
  SemanticsCustomAction FromFlutterSemanticsCustomAction(const FlutterSemanticsCustomAction* flutter_custom_action);
  FML_DISALLOW_COPY_AND_ASSIGN(AccessibilityBridge);
};

} // namespace ax

#endif
