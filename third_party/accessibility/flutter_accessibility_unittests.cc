// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "accessibility_bridge.h"
#include "ax/ax_action_data.h"
#include "test_flutter_accessibility.h"
#include "gtest/gtest.h"


namespace ax {
namespace testing {

TEST(FlutterAccessibilityTest, canPerfomActions) {
  // Set up a flutter accessibility node.
  TestFlutterAccessibility* accessibility = static_cast<TestFlutterAccessibility*>(FlutterAccessibility::Create());
  AccessibilityBridge bridge(nullptr);
  AXNode ax_node(bridge.GetAXTree(), 0, -1, -1);
  accessibility->Init(&bridge, &ax_node);

  // Performs an AXAction.
  AXActionData action_data;
  action_data.action = Action::kDoDefault;
  accessibility->AccessibilityPerformAction(action_data);
  ASSERT_EQ(accessibility->performed_actions.size(), size_t{1});
  ASSERT_EQ(accessibility->performed_actions[0], FlutterSemanticsAction::kFlutterSemanticsActionTap);

  action_data.action = Action::kFocus;
  accessibility->AccessibilityPerformAction(action_data);
  ASSERT_EQ(accessibility->performed_actions.size(), size_t{2});
  ASSERT_EQ(accessibility->performed_actions[1], FlutterSemanticsAction::kFlutterSemanticsActionDidGainAccessibilityFocus);

  action_data.action = Action::kScrollToMakeVisible;
  accessibility->AccessibilityPerformAction(action_data);
  ASSERT_EQ(accessibility->performed_actions.size(), size_t{3});
  ASSERT_EQ(accessibility->performed_actions[2], FlutterSemanticsAction::kFlutterSemanticsActionShowOnScreen);
}

}  // namespace testing
}  // namespace ax
