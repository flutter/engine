// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/AXPlatformNodeDelegateMac.h"

#include <functional>

#include "flutter/third_party/accessibility/ax/platform/ax_platform_node.h"

namespace flutter { // namespace

AXPlatformNodeDelegateMac::AXPlatformNodeDelegateMac(AXPlatformNodeDelegateOwner* bridge) {
  bridge_ = bridge;
  ax_platform_node_ = ax::AXPlatformNode::Create(this);
  FML_DCHECK(ax_platform_node_);
}

AXPlatformNodeDelegateMac::~AXPlatformNodeDelegateMac() {
  bridge_ = nullptr;
  ax_platform_node_->Destroy();
}

void AXPlatformNodeDelegateMac::UpdateWith(const FlutterSemanticsNode* flutter_node) {
  node_data_.id = flutter_node->id;
  node_data_.SetName(flutter_node->label);
  node_data_.relative_bounds.bounds.setLTRB(
    flutter_node->rect.left,
    flutter_node->rect.top,
    flutter_node->rect.right,
    flutter_node->rect.bottom
  );
  node_data_.relative_bounds.transform.setAll(
    flutter_node->transform.scaleX,
    flutter_node->transform.skewX,
    flutter_node->transform.transX,
    flutter_node->transform.skewY,
    flutter_node->transform.scaleY,
    flutter_node->transform.transY,
    flutter_node->transform.pers0,
    flutter_node->transform.pers1,
    flutter_node->transform.pers2
  );
  children_in_traversal_order_.clear();
  for (size_t i = 0; i < flutter_node->child_count; i++) {
    int32_t child = flutter_node->children_in_traversal_order[i];
    children_in_traversal_order_.push_back(child);
    // Points the child's parent to this node.
    bridge_->GetOrCreateAXPlatformNodeDelegate(child)->parent_ = flutter_node->id;
  }
  // TODO: assing real role
  node_data_.role = children_in_traversal_order_.size()==0? ax::Role::kStaticText : ax::Role::kGroup;
}

const std::vector<int32_t>& AXPlatformNodeDelegateMac::GetChildren() {
  return children_in_traversal_order_;
}

// AXPlatformNodeDelegateBase override
const ax::AXNodeData& AXPlatformNodeDelegateMac::GetData() const {
  return node_data_;
}

gfx::NativeViewAccessible AXPlatformNodeDelegateMac::GetNativeViewAccessible() {
  FML_DCHECK(ax_platform_node_);
  return ax_platform_node_->GetNativeViewAccessible();
}

gfx::NativeViewAccessible AXPlatformNodeDelegateMac::GetParent() {
  if (parent_ == kInvalidID) {
    return nullptr;
  }
  return bridge_->GetOrCreateAXPlatformNodeDelegate(parent_)->GetNativeViewAccessible();
}

SkRect AXPlatformNodeDelegateMac::GetBoundsRect(const ax::AXCoordinateSystem coordinate_system,
                     const ax::AXClippingBehavior clipping_behavior,
                     ax:: AXOffscreenResult* offscreen_result) const {
  // TODO: consider screen dpr and figureout what is offscreen_result.
  // switch (coordinate_system) {
  //   case ax::AXCoordinateSystem::kScreenPhysicalPixels:
  //     NSLog(@"ax::AXCoordinateSystem::kScreenPhysicalPixels");
  //     break;
  //   case ax::AXCoordinateSystem::kRootFrame:
  //     NSLog(@"ax::AXCoordinateSystem::kRootFrame");
  //     break;
  //   case ax::AXCoordinateSystem::kFrame:
  //     NSLog(@"ax::AXCoordinateSystem::kFrame");
  //     break;
  //   case ax::AXCoordinateSystem::kScreenDIPs:
  //     NSLog(@"ax::AXCoordinateSystem::kScreenDIPs");
  //     break;
  // }
  switch (coordinate_system) {
    case ax::AXCoordinateSystem::kScreenPhysicalPixels:
    case ax::AXCoordinateSystem::kRootFrame:
    case ax::AXCoordinateSystem::kFrame:
    case ax::AXCoordinateSystem::kScreenDIPs: {
      // Find all parent transforms.
      SkMatrix global_transform= node_data_.relative_bounds.transform;
      int32_t parent = parent_;
      while(parent != kInvalidID) {
        auto parent_delegate = bridge_->GetOrCreateAXPlatformNodeDelegate(parent);
        global_transform = parent_delegate->GetData().relative_bounds.transform * global_transform;
        parent = parent_delegate->parent_;
      }
      // Applies window transform.
      global_transform = bridge_->GetWindowTransform() * global_transform;
      SkRect result;
      global_transform.mapRect(&result, node_data_.relative_bounds.bounds);
      return result;
    }
  }
}

int AXPlatformNodeDelegateMac::GetChildCount() const {
  return static_cast<int>(children_in_traversal_order_.size());
}

gfx::NativeViewAccessible AXPlatformNodeDelegateMac::ChildAtIndex(int index) {
  int32_t child = children_in_traversal_order_[index];
  return bridge_->GetOrCreateAXPlatformNodeDelegate(child)->GetNativeViewAccessible();
}

}  // namespace flutter
