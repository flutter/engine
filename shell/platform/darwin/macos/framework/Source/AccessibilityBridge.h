// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_ACCESSIBILITYBRIDGE_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_ACCESSIBILITYBRIDGE_H_

#import <Cocoa/Cocoa.h>

#include <unordered_map>

#include "flutter/fml/macros.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterViewController.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/AXPlatformNodeDelegateMac.h"
// #import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
// #import "flutter/third_party/accessibility/ax/platform/ax_platform_node_delegate_base.h

namespace flutter {

class AccessibilityBridge : public AXPlatformNodeDelegateMac::AXPlatformNodeDelegateOwner {
 public:
  AccessibilityBridge(FlutterEngine* engine);
  ~AccessibilityBridge();

  void UpdateSemanticsNode(const FlutterSemanticsNode* node);
  AXPlatformNodeDelegateMac* GetOrCreateAXPlatformNodeDelegate(int32_t id) override;
//   void DispatchSemanticsAction(int32_t id, flutter::SemanticsAction action) override;
//   void DispatchSemanticsAction(int32_t id,
//                                flutter::SemanticsAction action,
//                                std::vector<uint8_t> args) override;
//   void AccessibilityObjectDidBecomeFocused(int32_t id) override;
//   void AccessibilityObjectDidLoseFocus(int32_t id) override;
//   FlutterUpdateSemanticsNodeCallback GetUpdateSemantics();
  const SkMatrix GetWindowTransform() override;
 private:
  std::unordered_map<int32_t, AXPlatformNodeDelegateMac*> objects_;
  FlutterEngine* engine_;
  void FinalizeSemanticsUpdate();
  void VisitObjectsRecursivelyAndRemove(int32_t node_id,
                                        std::set<int32_t>* doomed_uids);
  FML_DISALLOW_COPY_AND_ASSIGN(AccessibilityBridge);
};

} // namespace flutter

#endif
