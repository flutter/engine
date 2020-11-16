// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FlutterAccessibilityMac_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FlutterAccessibilityMac_H_

#import <Cocoa/Cocoa.h>

#import "flutter/shell/platform/embedder/embedder.h"
#include "flutter/third_party/accessibility/flutter_accessibility.h"

#define kRootNode 0

namespace ax {

class FlutterAccessibilityMac : public FlutterAccessibility {
 public:
  FlutterAccessibilityMac();
  ~FlutterAccessibilityMac();
  // FlutterAccessibility override
  void OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event) override;
  const AXNodeData& GetData() const override;
  void DispatchAccessibilityAction(uint16_t target, FlutterSemanticsAction action, uint8_t* data, size_t data_size) override;
  gfx::NativeViewAccessible GetNativeViewAccessible() override;
  gfx::NativeViewAccessible GetParent() override;
  int GetChildCount() const override;
  SkRect GetBoundsRect(const AXCoordinateSystem coordinate_system,
                          const AXClippingBehavior clipping_behavior,
                          AXOffscreenResult* offscreen_result) const override;
  gfx::NativeViewAccessible ChildAtIndex(int index) override;
  gfx::NativeViewAccessible GetNSWindow() override;
 
 private:
  AXNode* ax_node_;
  AXPlatformNode* ax_platform_node_;
  std::u16string old_text_editing_value_;
  std::string GetLiveRegionText() const;
  NSDictionary* GetUserInfoForValueChangedNotification(const id native_node, const std::u16string& deleted_text, const std::u16string& inserted_text, id edit_text_marker);
  FlutterEngine* GetFlutterEngine();
  void FireNativeMacNotification(gfx::NativeViewAccessible native_node, NSString* mac_notification);
  void FireNativeMacNotificationWithUserInfo(gfx::NativeViewAccessible native_node, NSString* mac_notification, NSDictionary* user_info);
  void computeTextEdit(std::u16string& inserted_text, std::u16string& deleted_text, id* edit_text_marker);
  bool IsInGeneratedEventBatch(ax::AXEventGenerator::Event event_type) const;
};

} // namespace ax

#endif
