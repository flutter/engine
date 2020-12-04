// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCESSIBILITY_ACCESSIBILITY_BRIDGE_H_
#define ACCESSIBILITY_ACCESSIBILITY_BRIDGE_H_

#include <unordered_map>

#include "flutter/shell/platform/embedder/embedder.h"

#include "ax/ax_event_generator.h"
#include "ax/ax_tree_observer.h"
#include "ax/ax_tree.h"
#include "ax/platform/ax_platform_node_delegate.h"

#include "flutter_accessibility.h"

namespace ax {

//------------------------------------------------------------------------------
/// Use this class to maintain an accessibility tree. This class consumes semantics
/// updates from the embedder API and produces an accessibility tree in the native
/// format.
///
/// To use this class, you must provide your own implementation of
/// FlutterAccessibility.
class AccessibilityBridge : public AXTreeObserver {
 public:
  //------------------------------------------------------------------------------
  /// @brief      Creates a new instance of a accessibility bridge.
  ///
  /// @param[in]  user_data           A custom pointer to the data of your
  ///                                 choice. This pointer can be retrieve later
  ///                                 through GetUserData().
  AccessibilityBridge(void* user_data);
  ~AccessibilityBridge();

  //------------------------------------------------------------------------------
  /// @brief      Adds a semantics node update to the pending semantics update.
  ///             Calling this method alone will NOT update the semantics tree.
  ///             To flush the pending updates, call the CommitUpdates().
  ///
  /// @param[in]  node           A pointer to the semantics node update.
  void AddFlutterSemanticsNodeUpdate(const FlutterSemanticsNode* node);

  //------------------------------------------------------------------------------
  /// @brief      Adds a custom semantics action update to the pending semantics
  ///             update. Calling this method alone will NOT update the
  ///             semantics tree. To flush the pending updates, call the
  ///             CommitUpdates().
  ///
  /// @param[in]  action           A pointer to the custom semantics action update.
  void AddFlutterSemanticsCustomActionUpdate(const FlutterSemanticsCustomAction* action);
  
  //------------------------------------------------------------------------------
  /// @brief      Flushes the pending updates and applies them to this
  ///             accessibility bridge.
  void CommitUpdates();

  //------------------------------------------------------------------------------
  /// @brief      Get the underlying AXTree.
  ax::AXTree* GetAXTree();

  //------------------------------------------------------------------------------
  /// @brief      The event generator of this accessibility bridge. It contains
  ///             the pending accessibility events generated as a result of a
  ///             semantics update.
  AXEventGenerator* GetEventGenerator();

  //------------------------------------------------------------------------------
  /// @brief      Get the user data.
  void* GetUserData();

  //------------------------------------------------------------------------------
  /// @brief      Get the flutter accessibility node with the given id from this
  ///             accessibility bridge.
  ///
  /// @param[in]  id           The id of the flutter accessibility node you want
  ///                          to retrieve.
  FlutterAccessibility* GetFlutterAccessibilityFromID(int32_t id) const;
  
  //------------------------------------------------------------------------------
  /// @brief      Update the currently focused flutter accessibility node.
  ///
  /// @param[in]  id           The id of the currently focused flutter
  ///                          accessibility node.
  void SetFocusedNode(int32_t node_id);

  //------------------------------------------------------------------------------
  /// @brief      Get the last focused node.
  int32_t GetLastFocusedNode();

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
  int32_t last_focused_node_ = ax::AXNode::kInvalidAXID;
  void* user_data_;

  void InitAXTree(const AXTreeUpdate& initial_state);
  void GetSubTreeList(SemanticsNode target, std::vector<SemanticsNode>& result);
  void ConvertFluterUpdate(const SemanticsNode& node, AXTreeUpdate& tree_update);
  void SetRoleFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetStateFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetActionsFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetBooleanAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetIntAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetIntListAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetStringListAttributesFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetNameFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetValueFromFlutterUpdate(AXNodeData& node_data, const SemanticsNode& node);
  void SetTreeData(const SemanticsNode& node, AXTreeUpdate& tree_update);
  SemanticsNode FromFlutterSemanticsNode(const FlutterSemanticsNode* flutter_node);
  SemanticsCustomAction FromFlutterSemanticsCustomAction(const FlutterSemanticsCustomAction* flutter_custom_action);
  FML_DISALLOW_COPY_AND_ASSIGN(AccessibilityBridge);
};

} // namespace ax

#endif // ACCESSIBILITY_ACCESSIBILITY_BRIDGE_H_
