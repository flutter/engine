// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/shell/platform/ios/accessibility_bridge.h"

#include <UIKit/UIKit.h>

namespace sky {
namespace shell {
namespace a11y {

AccessibilityBridge::AccessibilityBridge(
    FlutterView* view,
    semantics::SemanticsServerPtr& semanticsServer)
    : view_(view), binding_(this), weak_factory_(this) {
  mojo::InterfaceHandle<semantics::SemanticsListener> listener;
  binding_.Bind(&listener);
  semanticsServer->AddSemanticsListener(listener.Pass());
}

void AccessibilityBridge::UpdateSemanticsTree(
    mojo::Array<semantics::SemanticsNodePtr> nodes) {
  for (const semantics::SemanticsNodePtr& node : nodes) {
    UpdateNode(node);
  }

  NSArray* accessibleElements = CreateAccessibleElements();
  view_.accessibilityElements = accessibleElements;
  [accessibleElements release];

  UIAccessibilityPostNotification(UIAccessibilityLayoutChangedNotification,
                                  nil);
}

base::WeakPtr<AccessibilityBridge> AccessibilityBridge::AsWeakPtr() {
  return weak_factory_.GetWeakPtr();
}

NodePtr AccessibilityBridge::UpdateNode(
    const semantics::SemanticsNodePtr& node) {
  NodePtr persistentNode;
  NodeMap::iterator iter = nodes_.find(node->id);
  if (iter == nodes_.end()) {
    persistentNode = new Node(this, node);
    nodes_.insert(NodeMap::value_type(node->id, persistentNode));
  } else {
    persistentNode = iter->second;
    persistentNode->Update(node);
  }
  assert(persistentNode != nullptr);
  return persistentNode;
}

void AccessibilityBridge::RemoveNode(NodePtr node) {
  assert(nodes_.find(node->id_) != nodes_.end());
  assert(nodes_.at(node->id_)->parent_ == nullptr);
  nodes_.erase(node->id_);
  for (NodePtr& child : node->children_) {
    child->parent_ = nullptr;
    RemoveNode(child);
  }
}

NSArray* AccessibilityBridge::CreateAccessibleElements() const
    NS_RETURNS_RETAINED {
  NSMutableArray* accessibleElements = [[NSMutableArray alloc] init];
  for (const auto& iter : nodes_) {
    // TODO(tvolkert): n>1 roots will cause unstable sort here.
    // Verify that there should only ever be one root, and perhaps
    // store it so we don't have to search for it here.
    if (iter.second->parent_ == nullptr) {
      iter.second->PopulateAccessibleElements(accessibleElements);
    }
  }
  return accessibleElements;
}

AccessibilityBridge::~AccessibilityBridge() {}

Node::Node(AccessibilityBridge* bridge, const semantics::SemanticsNodePtr& node)
    : bridge_(bridge) {
  Update(node);
}

void Node::Update(const semantics::SemanticsNodePtr& node) {
  if (id_ == kUninitializedNodeId) {
    id_ = node->id;
  }
  assert(id_ == node->id);

  flags_ = node->flags;
  strings_ = node->strings;
  geometry_ = node->geometry;

  if (!node->children.is_null()) {
    const NodeList oldChildren = NodeList(children_);
    children_.clear();
    for (NodePtr child : oldChildren) {
      assert(child->parent_ != nullptr);
      child->parent_ = nullptr;
    }
    for (const semantics::SemanticsNodePtr& childNode : node->children) {
      NodePtr child = bridge_->UpdateNode(childNode);
      child->parent_ = this;
      children_.push_back(child);
    }
    for (NodePtr child : oldChildren) {
      if (child->parent_ == nullptr) {
        bridge_->RemoveNode(child);
      }
    }
  }

  global_transform_.release();
  global_rect_.release();
}

void Node::ValidateGlobalTransform() {
  if (global_transform_ != nullptr) {
    return;
  }

  if (parent_ == nullptr) {
    global_transform_.reset(new SkMatrix44(geometry_.transform));
  } else {
    parent_->ValidateGlobalTransform();
    const std::unique_ptr<SkMatrix44>& parentTransform =
        parent_->global_transform_;
    if (geometry_.transform.isIdentity()) {
      global_transform_.reset(new SkMatrix44(*parentTransform));
    } else if (parentTransform->isIdentity()) {
      global_transform_.reset(new SkMatrix44(geometry_.transform));
    } else {
      global_transform_.reset(
          new SkMatrix44(geometry_.transform * *parentTransform));
    }
  }
}

void Node::ValidateGlobalRect() {
  if (global_rect_ != nullptr) {
    return;
  }

  ValidateGlobalTransform();

  SkPoint quad[4];
  geometry_.rect.toQuad(quad);
  for (auto& point : quad) {
    SkScalar vector[4] = {point.x(), point.y(), 0, 1};
    global_transform_->mapScalars(vector);
    point.set(vector[0], vector[1]);
  }

  global_rect_.reset(new SkRect());
  global_rect_.get()->set(quad, 4);
}

void Node::PopulateAccessibleElements(NSMutableArray* accessibleElements) {
  if (!geometry_.rect.isEmpty()) {
    UIAccessibilityElement* element = [[UIAccessibilityElement alloc]
        initWithAccessibilityContainer:bridge_->view_];
    element.isAccessibilityElement = YES;
    ValidateGlobalRect();
    element.accessibilityFrame =
        CGRectMake(global_rect_->x(), global_rect_->y(), global_rect_->width(),
                   global_rect_->height());
    if (flags_.can_be_tapped) {
      // TODO(tvolkert): What about links? We need semantic info in the mojom
      // definition
      element.accessibilityTraits = UIAccessibilityTraitButton;
    }
    if (!strings_.label.empty()) {
      element.accessibilityLabel =
          [NSString stringWithUTF8String:strings_.label.data()];
    }
    [accessibleElements insertObject:element atIndex:0];
    [element release];
  }

  for (NodePtr child : children_) {
    child->PopulateAccessibleElements(accessibleElements);
  }
}

Node::~Node() {}
}  // namespace a11y
}  // namespace shell
}  // namespace sky
