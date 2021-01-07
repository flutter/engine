// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CPP_TEST_ACCESSIBILITY_BRIDGE_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CPP_TEST_ACCESSIBILITY_BRIDGE_H_

#include "accessibility_bridge.h"

namespace ui {

class TestAccessibilityBridgeDelegate
    : public AccessibilityBridge::AccessibilityBridgeDelegate {
 public:
  TestAccessibilityBridgeDelegate() = default;

  void OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event,
                            AccessibilityBridge* bridge) override;
  void DispatchAccessibilityAction(uint16_t target,
                                   FlutterSemanticsAction action,
                                   uint8_t* data,
                                   size_t data_size) override;

  std::vector<AXEventGenerator::TargetedEvent> accessibilitiy_events;
  std::vector<FlutterSemanticsAction> performed_actions;
};

}  // namespace ui

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CPP_TEST_ACCESSIBILITY_BRIDGE_H_
