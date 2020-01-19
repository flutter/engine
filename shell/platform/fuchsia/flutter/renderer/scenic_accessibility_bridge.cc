// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_accessibility_bridge.h"

#include <fuchsia/ui/gfx/cpp/fidl.h>
#include <zircon/status.h>
#include <zircon/types.h>

#include <deque>

#include "flutter/shell/platform/fuchsia/utils/logging.h"

namespace flutter_runner {
namespace {

constexpr int32_t kRootNodeId = 0;

// The only known usage of a negative number for a node ID is in the embedder
// API as a sentinel value, which is not expected here. No valid producer of
// nodes should give us a negative ID.
uint32_t FlutterIdToFuchsiaId(int32_t flutter_node_id) {
  FX_DCHECK(flutter_node_id >= 0)
      << "Unexpectedly recieved a negative semantics node ID.";
  return static_cast<uint32_t>(flutter_node_id);
}

fuchsia::ui::gfx::BoundingBox GetNodeLocation(
    const FlutterSemanticsNode* node) {
  fuchsia::ui::gfx::BoundingBox box;
  box.min.x = node->rect.left;
  box.min.y = node->rect.top;
  box.min.z = static_cast<float>(node->elevation);
  box.max.x = node->rect.right;
  box.max.y = node->rect.bottom;
  box.max.z = static_cast<float>(node->thickness);
  return box;
}

fuchsia::ui::gfx::mat4 GetNodeTransform(const FlutterSemanticsNode* node) {
  /* From Skia:
   * [ scX skX trX ]      [ scX skX 0 trX ]
   * [ skY scY trY ]  ->  [ skY scY 0 trY ]
   * [ ps0 ps1 ps2 ]      [ 0 0 1 0 ]
   *                      [ ps0 ps1 0 ps2 ]
   */
  return fuchsia::ui::gfx::mat4{
      .matrix = {static_cast<float>(node->transform.scaleX),
                 static_cast<float>(node->transform.skewX), 0.0f,
                 static_cast<float>(node->transform.transX),
                 static_cast<float>(node->transform.skewY),
                 static_cast<float>(node->transform.scaleY), 0.0f,
                 static_cast<float>(node->transform.transY), 0.0f, 0.0f, 1.0f,
                 0.0f, static_cast<float>(node->transform.pers0),
                 static_cast<float>(node->transform.pers1), 0.0f,
                 static_cast<float>(node->transform.pers2)},
  };
}

fuchsia::accessibility::semantics::Attributes GetNodeAttributes(
    const FlutterSemanticsNode* node,
    size_t* added_size) {
  // TODO(MI4-2531): Don't truncate.
  auto label = std::string(node->label,
                           fuchsia::accessibility::semantics::MAX_LABEL_SIZE);
  *added_size += label.size();

  fuchsia::accessibility::semantics::Attributes attributes;
  attributes.set_label(std::move(label));

  return attributes;
}

fuchsia::accessibility::semantics::States GetNodeStates(
    const FlutterSemanticsNode* node) {
  fuchsia::accessibility::semantics::States states;
  if (node->flags & kFlutterSemanticsFlagHasCheckedState) {
    states.set_checked(node->flags & kFlutterSemanticsFlagIsChecked);
  }

  return states;
}

}  // end namespace

ScenicAccessibilityBridge::ScenicAccessibilityBridge(
    Renderer::DispatchTable dispatch_table,
    fuchsia::ui::views::ViewRef view_ref,
    std::shared_ptr<sys::ServiceDirectory> services)
    : dispatch_table_(dispatch_table), binding_(this) {
  fuchsia_semantics_manager_.set_error_handler([this](zx_status_t status) {
    FX_LOG(ERROR)
        << "Interface error for "
           "fuchsia::accessibility::semantics::SemanticsManager with status: "
        << zx_status_get_string(status);
    dispatch_table_.error_callback();
  });
  binding_.set_error_handler([this](zx_status_t status) {
    FX_LOG(ERROR)
        << "Interface error (binding) for "
           "fuchsia::accessibility::semantics::SemanticListener with status: "
        << zx_status_get_string(status);
    dispatch_table_.error_callback();
  });

  services->Connect(fuchsia::accessibility::semantics::SemanticsManager::Name_,
                    fuchsia_semantics_manager_.NewRequest().TakeChannel());

  fidl::InterfaceHandle<fuchsia::accessibility::semantics::SemanticListener>
      listener_handle;
  binding_.Bind(listener_handle.NewRequest());
  fuchsia_semantics_manager_->RegisterViewForSemantics(
      std::move(view_ref), std::move(listener_handle), tree_ptr_.NewRequest());
}

bool ScenicAccessibilityBridge::GetSemanticsEnabled() const {
  return semantics_enabled_;
}

void ScenicAccessibilityBridge::SetSemanticsEnabled(bool enabled) {
  semantics_enabled_ = enabled;
  if (!enabled) {
    nodes_.clear();
  }
}

void ScenicAccessibilityBridge::AddSemanticsNodeUpdate(
    const FlutterSemanticsNode* node) {
  FX_DCHECK(nodes_.find(kRootNodeId) != nodes_.end() || node->id == kRootNodeId)
      << "ScenicAccessibilityBridge received an update without ever getting a "
         "root node";

  // Check for the sentinel node which indicates that updates have finished;
  // commit all of the updates when found.
  if (node->id == -1) {
    PruneUnreachableNodes();
    FlushNodeUpdates();

    // TODO(dnfield): Implement the callback here
    // https://bugs.fuchsia.dev/p/fuchsia/issues/detail?id=35718.
    tree_ptr_->CommitUpdates([]() {});

    return;
  }
  FX_DCHECK(node->id >= 0);  // -1 is the only valid negative value

  // TODO(MI4-2498): Actions, Roles, hit test children, additional
  // flags/states/attr
  // TODO(MI4-1478): Support for partial updates for nodes > 64kb
  // e.g. if a node has a long label or more than 64k children.
  fuchsia::accessibility::semantics::Node fuchsia_node;
  size_t this_node_size = sizeof(fuchsia::accessibility::semantics::Node);
  std::vector<uint32_t> fuchsia_child_ids(node->child_count);

  // Handle caching and conversion of child node IDs.
  for (size_t i = 0; i < node->child_count; i++) {
    uint32_t child_id =
        FlutterIdToFuchsiaId(node->children_in_traversal_order[i]);
    fuchsia_child_ids.push_back(child_id);
  }

  // Convert to Fuchsia node, and cache children IDs.
  uint32_t fuchsia_node_id = FlutterIdToFuchsiaId(node->id);
  fuchsia_node.set_node_id(fuchsia_node_id)
      .set_location(GetNodeLocation(node))
      .set_transform(GetNodeTransform(node))
      .set_attributes(GetNodeAttributes(node, &this_node_size))
      .set_states(GetNodeStates(node))
      .set_child_ids(fuchsia_child_ids);
  nodes_[fuchsia_node_id] = std::vector<int32_t>(
      node->children_in_traversal_order,
      node->children_in_traversal_order + node->child_count);
  this_node_size += kNodeIdSize * nodes_[fuchsia_node_id].size();

  // TODO(MI4-2531, FIDL-718): Remove this, handle the error instead in
  // something like set_error_handler.
  // This is defensive. If, despite our best efforts, we ended up with a node
  // that is larger than the max fidl size, we send no updates.
  if (this_node_size >= kMaxMessageSize) {
    FX_LOG(ERROR) << "Semantics node with ID " << node->id
                  << " exceeded the maximum FIDL message size and may not be "
                     "delivered to the accessibility manager service.";
    return;
  }

  // If we would exceed the max FIDL message size by appending this node,
  // we should commit now.
  if (pending_message_size_ + this_node_size >= kMaxMessageSize) {
    FlushNodeUpdates();
  }

  // Stash the node for committing later.
  pending_nodes_.push_back(std::move(fuchsia_node));
  pending_message_size_ += this_node_size;
}

void ScenicAccessibilityBridge::AddSemanticsCustomActionUpdate(
    const FlutterSemanticsCustomAction* action) {}

void ScenicAccessibilityBridge::HitTest(
    fuchsia::math::PointF local_point,
    fuchsia::accessibility::semantics::SemanticListener::HitTestCallback
        callback) {}

void ScenicAccessibilityBridge::OnAccessibilityActionRequested(
    uint32_t node_id,
    fuchsia::accessibility::semantics::Action action,
    fuchsia::accessibility::semantics::SemanticListener::
        OnAccessibilityActionRequestedCallback callback) {}

void ScenicAccessibilityBridge::OnSemanticsModeChanged(
    bool enabled,
    OnSemanticsModeChangedCallback callback) {
  if (enabled) {
    dispatch_table_.update_accessibility_features_callback(
        kFlutterAccessibilityFeatureAccessibleNavigation);
  } else {
    const FlutterAccessibilityFeature kFlutterAccessibilityFeatureEmpty =
        static_cast<FlutterAccessibilityFeature>(0);
    dispatch_table_.update_accessibility_features_callback(
        kFlutterAccessibilityFeatureEmpty);
  }
}

std::unordered_set<int32_t> ScenicAccessibilityBridge::GetDescendants(
    int32_t node_id) const {
  std::unordered_set<int32_t> descendents;
  std::deque<int32_t> to_process = {node_id};
  while (!to_process.empty()) {
    int32_t id = to_process.front();
    to_process.pop_front();
    descendents.emplace(id);

    auto it = nodes_.find(id);
    if (it != nodes_.end()) {
      auto const& children = it->second;
      for (const auto& child : children) {
        if (descendents.find(child) == descendents.end()) {
          to_process.push_back(child);
        } else {
          // This indicates either a cycle or a child with multiple parents.
          // Flutter should never let this happen, but the engine API does not
          // explicitly forbid it right now.
          FX_LOG(ERROR) << "Semantics Node " << child
                        << " has already been listed as a child of another "
                           "node, ignoring for parent "
                        << id << ".";
        }
      }
    }
  }
  return descendents;
}

void ScenicAccessibilityBridge::FlushNodeUpdates() {
  tree_ptr_->UpdateSemanticNodes(std::move(pending_nodes_));
  pending_nodes_.clear();
  pending_message_size_ = 0;
}

void ScenicAccessibilityBridge::PruneUnreachableNodes() {
  const auto& reachable_nodes = GetDescendants(kRootNodeId);
  std::vector<uint32_t> nodes_to_remove;
  auto iter = nodes_.begin();
  while (iter != nodes_.end()) {
    int32_t id = iter->first;
    if (reachable_nodes.find(id) == reachable_nodes.end()) {
      // TODO(MI4-2531): This shouldn't be strictly necessary at this level.
      if (sizeof(nodes_to_remove) + (nodes_to_remove.size() * kNodeIdSize) >=
          kMaxMessageSize) {
        tree_ptr_->DeleteSemanticNodes(std::move(nodes_to_remove));
        nodes_to_remove.clear();
      }
      nodes_to_remove.push_back(FlutterIdToFuchsiaId(id));
      iter = nodes_.erase(iter);
    } else {
      iter++;
    }
  }
  if (!nodes_to_remove.empty()) {
    tree_ptr_->DeleteSemanticNodes(std::move(nodes_to_remove));
  }
}

}  // namespace flutter_runner
