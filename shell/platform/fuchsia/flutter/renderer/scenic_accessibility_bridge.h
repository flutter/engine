// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RENDERER_SCENIC_ACCESSIBILITY_BRIDGE_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RENDERER_SCENIC_ACCESSIBILITY_BRIDGE_H_

#include <fuchsia/accessibility/semantics/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/fidl/cpp/binding.h>
#include <lib/sys/cpp/service_directory.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer.h"

namespace flutter_runner {

// This embedder component intermediates accessibility-related calls between
// Fuchsia and Flutter. It serves to resolve the impedance mismatch between
// Flutter's platform-agnostic accessibility APIs and Fuchsia's APIs and
// behaviour.
//
// The bridge performs the following functions, among others:
//
// * Translates Flutter's semantics node updates to events Fuchsia requires
//   (e.g. Flutter only sends updates for changed nodes, but Fuchsia requires
//   the entire flattened subtree to be sent when a node changes.
class ScenicAccessibilityBridge
    : public fuchsia::accessibility::semantics::SemanticListener {
 public:
  // TODO(MI4-2531, FIDL-718): Remove this. We shouldn't be worried about
  // batching messages at this level.
  // FIDL may encode a C++ struct as larger than the sizeof the C++ struct.
  // This is to make sure we don't send updates that are too large.
  static constexpr uint32_t kMaxMessageSize = ZX_CHANNEL_MAX_MSG_BYTES / 2;
  static_assert(fuchsia::accessibility::semantics::MAX_LABEL_SIZE <
                kMaxMessageSize - 1);

  // Flutter uses signed 32 bit integers for node IDs, while Fuchsia uses
  // unsigned 32 bit integers. A change in the size on either one would break
  // casts and size tracking logic in the implementation.
  static constexpr size_t kNodeIdSize = sizeof(FlutterSemanticsNode::id);
  static_assert(
      kNodeIdSize ==
          sizeof(fuchsia::accessibility::semantics::Node().node_id()),
      "FlutterSemanticsNode::id and "
      "fuchsia::accessibility::semantics::Node::node_id differ in size.");

  ScenicAccessibilityBridge(Renderer::DispatchTable dispatch_table,
                            fuchsia::ui::views::ViewRef view_ref,
                            std::shared_ptr<sys::ServiceDirectory> services);
  ScenicAccessibilityBridge(const ScenicAccessibilityBridge&) = delete;
  ScenicAccessibilityBridge(ScenicAccessibilityBridge&&) = delete;
  ~ScenicAccessibilityBridge() = default;

  ScenicAccessibilityBridge& operator=(const ScenicAccessibilityBridge&) =
      delete;
  ScenicAccessibilityBridge& operator=(ScenicAccessibilityBridge&&) = delete;

  // Returns true if accessible navigation is enabled.
  bool GetSemanticsEnabled() const;

  // Enables Flutter accessibility navigation features.
  //
  // Once enabled, any semantics updates in the Flutter application will
  // trigger |FuchsiaAccessibility::DispatchAccessibilityEvent| callbacks
  // to send events back to the Fuchsia SemanticsManager.
  void SetSemanticsEnabled(bool enabled);

  // Adds a semantics node update to the buffer of node updates to apply.
  void AddSemanticsNodeUpdate(const FlutterSemanticsNode* node);

  // Adds a semantics custom action update to the buffer of custom action
  // updates to apply.
  void AddSemanticsCustomActionUpdate(
      const FlutterSemanticsCustomAction* action);

 private:
  // |fuchsia::accessibility::semantics::SemanticListener|
  void HitTest(
      fuchsia::math::PointF local_point,
      fuchsia::accessibility::semantics::SemanticListener::HitTestCallback
          callback) override;

  // |fuchsia::accessibility::semantics::SemanticListener|
  void OnAccessibilityActionRequested(
      uint32_t node_id,
      fuchsia::accessibility::semantics::Action action,
      fuchsia::accessibility::semantics::SemanticListener::
          OnAccessibilityActionRequestedCallback callback) override;

  // |fuchsia::accessibility::semantics::SemanticListener|
  void OnSemanticsModeChanged(bool enabled,
                              OnSemanticsModeChangedCallback callback) override;

  // Gets the set of reachable descendants from the given node id.
  std::unordered_set<int32_t> GetDescendants(int32_t node_id) const;

  // Send a list of Node updates to the Fuchsia semantics manager as a batch.
  void FlushNodeUpdates();

  // Removes internal references to any dangling nodes from previous
  // updates, and updates the Accessibility service.
  //
  // May result in a call to FuchsiaAccessibility::Commit().
  void PruneUnreachableNodes();

  Renderer::DispatchTable dispatch_table_;

  fidl::Binding<fuchsia::accessibility::semantics::SemanticListener> binding_;

  fuchsia::accessibility::semantics::SemanticsManagerPtr
      fuchsia_semantics_manager_;
  fuchsia::accessibility::semantics::SemanticTreePtr tree_ptr_;

  // This is the cache of all nodes we've sent to Fuchsia's SemanticsManager.
  // Assists with pruning unreachable nodes and hit testing.
  std::unordered_map<int32_t, std::vector<int32_t>> nodes_;

  // This is a list of notes yet to be sent to Fuchsia's SemanticsManager.
  // Nodes are batched up and sent as a group for efficiency's sake.
  std::vector<fuchsia::accessibility::semantics::Node> pending_nodes_;
  size_t pending_message_size_ = 0;

  bool semantics_enabled_;
};
}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_RENDERER_SCENIC_ACCESSIBILITY_BRIDGE_H_
