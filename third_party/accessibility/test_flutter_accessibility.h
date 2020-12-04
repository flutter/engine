// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCESSIBILITY_TEST_FLUTTER_ACCESSIBILITY_H
#define ACCESSIBILITY_TEST_FLUTTER_ACCESSIBILITY_H

#include "flutter_accessibility.h"

namespace ax {

class TestFlutterAccessibility : public FlutterAccessibility {
 public:
  void OnAccessibilityEvent(AXEventGenerator::TargetedEvent targeted_event) override;
  void DispatchAccessibilityAction(uint16_t target, FlutterSemanticsAction action, uint8_t* data, size_t data_size) override;

  std::vector<AXEventGenerator::TargetedEvent> accessibilitiy_events;
  std::vector<FlutterSemanticsAction> performed_actions;
};

} // namespace ax

#endif // ACCESSIBILITY_TEST_FLUTTER_ACCESSIBILITY_H
