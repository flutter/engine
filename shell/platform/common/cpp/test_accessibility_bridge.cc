// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "test_accessibility_bridge.h"

namespace flutter {

std::unique_ptr<FlutterAccessibility>
TestAccessibilityBridgeDelegate::CreateFlutterAccessibility() {
  return std::make_unique<FlutterAccessibility>();
};

void TestAccessibilityBridgeDelegate::OnAccessibilityEvent(
    ui::AXEventGenerator::TargetedEvent targeted_event,
    AccessibilityBridge* bridge) {
  accessibilitiy_events.push_back(targeted_event);
}

void TestAccessibilityBridgeDelegate::DispatchAccessibilityAction(
    uint16_t target,
    FlutterSemanticsAction action,
    std::unique_ptr<uint8_t[]> data,
    size_t data_size) {
  performed_actions.push_back(action);
}

}  // namespace flutter
