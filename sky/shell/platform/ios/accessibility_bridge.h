// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_SHELL_PLATFORM_IOS_SEMANTICS_LISTENER_IMPL_H_
#define SKY_SHELL_PLATFORM_IOS_SEMANTICS_LISTENER_IMPL_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "base/macros.h"
#include "base/memory/ref_counted.h"
#include "base/memory/weak_ptr.h"
#include "mojo/public/cpp/bindings/array.h"
#include "mojo/public/cpp/bindings/strong_binding.h"
#include "sky/engine/platform/geometry/FloatRect.h"
#include "sky/services/semantics/semantics.mojom.h"
#include "sky/shell/platform/ios/FlutterView.h"
#include "third_party/skia/include/core/SkRect.h"
#include "third_party/skia/include/utils/SkMatrix44.h"

namespace sky {
namespace shell {
namespace a11y {

class Node;

typedef scoped_refptr<Node> NodePtr;

// Class that listens for updates to the semantic tree from Dart code and issues
// corresponding updates to the FlutterView's accessibility elements.
class AccessibilityBridge final : public semantics::SemanticsListener {
 public:
  AccessibilityBridge(FlutterView*, semantics::SemanticsServerPtr&);
  ~AccessibilityBridge() override;

  // Listener method invoked from Dart that signals that the specified semantic
  // nodes have been updated in the render tree.
  void UpdateSemanticsTree(mojo::Array<semantics::SemanticsNodePtr>) override;

  base::WeakPtr<AccessibilityBridge> AsWeakPtr();

 private:
  typedef std::map<long, NodePtr> NodeMap;
  friend class Node;

  NodePtr UpdateNode(const semantics::SemanticsNodePtr& node);
  void RemoveNode(NodePtr node);

  NSArray* CreateAccessibleElements() const NS_RETURNS_RETAINED;

  // We expect to have the same lifetime as the view
  FlutterView* view_;
  NodeMap nodes_;

  mojo::StrongBinding<semantics::SemanticsListener> binding_;

  base::WeakPtrFactory<AccessibilityBridge> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(AccessibilityBridge);
};

struct Flags {
  Flags()
      : can_be_tapped(false),
        can_be_long_pressed(false),
        can_be_scrolled_horizontally(false),
        can_be_scrolled_vertically(false),
        has_checked_state(false),
        is_checked(false) {}

  // Assigns the specified flags to this struct iff it is non-null.
  // It is important not to update our flags when the semantic flags
  // are null, because a null update from Dart means "nothing to update"
  Flags& operator=(const semantics::SemanticFlagsPtr& other) {
    if (!other.is_null()) {
      can_be_tapped = other->canBeTapped;
      can_be_long_pressed = other->canBeLongPressed;
      can_be_scrolled_horizontally = other->canBeScrolledHorizontally;
      can_be_scrolled_vertically = other->canBeScrolledVertically;
      has_checked_state = other->hasCheckedState;
      is_checked = other->isChecked;
    }
    return *this;
  }

  bool can_be_tapped : 1;
  bool can_be_long_pressed : 1;
  bool can_be_scrolled_horizontally : 1;
  bool can_be_scrolled_vertically : 1;
  bool has_checked_state : 1;
  bool is_checked : 1;
};

struct Strings {
  // Assigns the specified strings to this struct iff it is non-null
  // It is important not to update our strings when the semantic strings
  // are null, because a null update from Dart means "nothing to update"
  Strings& operator=(const semantics::SemanticStringsPtr& other) {
    if (!other.is_null()) {
      if (!other->label.is_null()) {
        label = other->label.get();
      }
    }
    return *this;
  }

  std::string label = "";
};

struct Geometry {
  // Assigns the specified geometry to this struct iff it is non-null
  // It is important not to update our values when the semantic geometry
  // is null, because a null update from Dart means "nothing to update"
  Geometry& operator=(const semantics::SemanticGeometryPtr& other) {
    if (!other.is_null()) {
      if (!other->transform.is_null()) {
        transform.setColMajorf(other->transform.data());
      }
      rect.setXYWH(other->left, other->top, other->width, other->height);
    }
    return *this;
  }

  SkMatrix44 transform = SkMatrix44();
  SkRect rect = SkRect();
};

// Class that holds information about accessibility nodes, which are used
// to construct iOS accessibility elements
class Node final : public base::RefCounted<Node> {
 public:
  static const uint32_t kUninitializedNodeId = -1;

  void Update(const semantics::SemanticsNodePtr& node);
  void PopulateAccessibleElements(NSMutableArray* accessibleElements);

 private:
  friend class base::RefCounted<Node>;
  friend class AccessibilityBridge;
  typedef std::vector<NodePtr> NodeList;

  Node() = delete;
  Node(AccessibilityBridge*, const semantics::SemanticsNodePtr&);
  ~Node();

  void ValidateGlobalRect();
  void ValidateGlobalTransform();

  AccessibilityBridge* bridge_;
  Flags flags_;
  Strings strings_;
  Geometry geometry_;

  uint32_t id_ = kUninitializedNodeId;
  NodeList children_;

  // Raw pointer to avoid circular reference and since we don't own the parent
  Node* parent_ = nullptr;

  std::unique_ptr<SkMatrix44> global_transform_;
  std::unique_ptr<SkRect> global_rect_;

  DISALLOW_COPY_AND_ASSIGN(Node);
};

}  // namespace a11y
}  // namespace shell
}  // namespace sky

#endif  // SKY_SHELL_PLATFORM_IOS_SEMANTICS_LISTENER_IMPL_H_
