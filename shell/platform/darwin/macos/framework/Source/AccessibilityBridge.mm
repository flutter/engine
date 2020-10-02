// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/AccessibilityBridge.h"
#include <functional>
#include <utility>

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterAppDelegate.h"
#include "flutter/fml/logging.h"

namespace flutter { // namespace

// AccessibilityBridge
AccessibilityBridge::AccessibilityBridge(FlutterEngine* engine) {
  engine_ = engine;
}

AccessibilityBridge::~AccessibilityBridge() {
  engine_ = nullptr;
}

void AccessibilityBridge::UpdateSemanticsNode(const FlutterSemanticsNode* node) {
  if (node->id == kFlutterSemanticsNodeIdBatchEnd) {
    FinalizeSemanticsUpdate();
    return;
  }
  AXPlatformNodeDelegateMac* platform_delegate = GetOrCreateAXPlatformNodeDelegate(node->id);
  platform_delegate->UpdateWith(node);

}

AXPlatformNodeDelegateMac* AccessibilityBridge::GetOrCreateAXPlatformNodeDelegate(int32_t node_id) {
  auto it = objects_.find(node_id);
  if (it != objects_.end()) {
    return it->second;
  }
  AXPlatformNodeDelegateMac* new_node = new AXPlatformNodeDelegateMac(this);
  objects_.emplace(node_id, new_node);
  return new_node;
}

const SkMatrix AccessibilityBridge::GetWindowTransform() {
  // TODO(chunhtai): we need to find a better way to get get the window transform
  FlutterAppDelegate* appDelegate = (FlutterAppDelegate*)[NSApp delegate];
  NSRect rect = appDelegate.mainFlutterWindow.frame;
  // NSLog(@"the window rect %@", NSStringFromRect(rect));
  SkMatrix result;
  result.setTranslate(rect.origin.x, rect.origin.y);
  return result;
}

// void printTree(NSAccessibilityElement* element, std::vector<std::string>& msg,int level){
//   // msg[level] += std::string([element.accessibilityValue UTF8String]) + ", ";
//   NSLog(@"in print tree for level %d accessibilityValue %@, screen bound origin %@, size %@, enabled %d, accessibilityElement %d",level, element.accessibilityValue, NSStringFromPoint(element.accessibilityFrame.origin), NSStringFromSize(element.accessibilityFrame.size), element.accessibilityEnabled, element.accessibilityElement);
//   for (id child in element.accessibilityChildren) {
//     printTree(child, msg, level+1);
//   }
// }

void AccessibilityBridge::FinalizeSemanticsUpdate() {
  auto it = objects_.find(kRootNode); 
  if (it == objects_.end()) {
    // TODO(chunhtai): figure out whether we should also remove it from the root
    // also
    objects_.clear();
    return;
  }
  
  std::set<int32_t> doomed_uids;
  for(auto kv : objects_) {
    doomed_uids.insert(kv.first);
  }
  VisitObjectsRecursivelyAndRemove(kRootNode, &doomed_uids);
  for (int32_t uid : doomed_uids) {
    delete objects_[uid];
    objects_.erase(uid);
  }
  engine_.viewController.view.accessibilityChildren = @[it->second->GetNativeViewAccessible()];
  NSAccessibilityElement* root = it->second->GetNativeViewAccessible();
  // NSLog(@"root node: children count %d, children %@", it->second->GetChildCount(), root.accessibilityChildren);
  NSAccessibilityPostNotification(it->second->GetNativeViewAccessible(), NSAccessibilityLayoutChangedNotification);
  // std::vector<std::string> msg = {"", "", "", "", "", "","","","",""};
  // printTree(engine_.viewController.view.accessibilityChildren[0], msg,0);
  // for (auto m : msg){
  //   NSLog(@"%@", @(m.data()));
  // }
  // NSLog(@"update finish %@", engine_.viewController.view.accessibilityChildren);

}

void AccessibilityBridge::VisitObjectsRecursivelyAndRemove(int32_t node_id,
                                                           std::set<int32_t>* doomed_uids) {
  doomed_uids->erase(node_id);
  auto it = objects_.find(node_id);
  FML_DCHECK(it != objects_.end());
  AXPlatformNodeDelegateMac* node = it->second;
  for (int32_t child : node->GetChildren())
    VisitObjectsRecursivelyAndRemove(child, doomed_uids);
}

}  // namespace flutter
