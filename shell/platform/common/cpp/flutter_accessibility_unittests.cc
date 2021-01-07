// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_accessibility.h"

#include "flutter/third_party/accessibility/ax/ax_action_data.h"
#include "gtest/gtest.h"

#include "test_accessibility_bridge.h"

namespace ui {
namespace testing {

TEST(FlutterAccessibilityTest, canPerfomActions) {
  // Set up a flutter accessibility node.
  FlutterAccessibility* accessibility = FlutterAccessibility::Create();
  AccessibilityBridge bridge(std::make_unique<TestAccessibilityBridgeDelegate>(), nullptr);
  TestAccessibilityBridgeDelegate* delegate = (TestAccessibilityBridgeDelegate*)bridge.GetDelegate();

  AXNode ax_node(bridge.GetAXTree(), 0, -1, -1);
  accessibility->Init(&bridge, &ax_node);

  // Performs an AXAction.
  AXActionData action_data;
  action_data.action = ax::mojom::Action::kDoDefault;
  accessibility->AccessibilityPerformAction(action_data);
  ASSERT_EQ(delegate->performed_actions.size(), size_t{1});
  ASSERT_EQ(delegate->performed_actions[0],
            FlutterSemanticsAction::kFlutterSemanticsActionTap);

  action_data.action = ax::mojom::Action::kFocus;
  accessibility->AccessibilityPerformAction(action_data);
  ASSERT_EQ(delegate->performed_actions.size(), size_t{2});
  ASSERT_EQ(
      delegate->performed_actions[1],
      FlutterSemanticsAction::kFlutterSemanticsActionDidGainAccessibilityFocus);

  action_data.action = ax::mojom::Action::kScrollToMakeVisible;
  accessibility->AccessibilityPerformAction(action_data);
  ASSERT_EQ(delegate->performed_actions.size(), size_t{3});
  ASSERT_EQ(delegate->performed_actions[2],
            FlutterSemanticsAction::kFlutterSemanticsActionShowOnScreen);
}

}  // namespace testing
}  // namespace ui
