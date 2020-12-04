// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "flutter/testing/testing.h"

#include "flutter/third_party/accessibility/accessibility_bridge.h"
#include "flutter/third_party/accessibility/ax/ax_action_data.h"

#import "flutter/shell/platform/darwin/macos/framework/Headers/FlutterEngine.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterAccessibilityMac.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterDartProject_Internal.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterEngine_Internal.h"
#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"

namespace flutter::testing {

namespace {
// Returns an engine configured for the text fixture resource configuration.
FlutterEngine* CreateTestEngine() {
  NSString* fixtures = @(testing::GetFixturesPath());
  FlutterDartProject* project = [[FlutterDartProject alloc]
      initWithAssetsPath:fixtures
             ICUDataPath:[fixtures stringByAppendingString:@"/icudtl.dat"]];
  return [[FlutterEngine alloc] initWithName:@"test" project:project allowHeadlessExecution:true];
}
}  // namespace

TEST(FlutterAccessibilityMac, Basics) {
  FlutterEngine* engine = CreateTestEngine();
  ax::FlutterAccessibilityMac* accessibility =
      static_cast<ax::FlutterAccessibilityMac*>(ax::FlutterAccessibility::Create());
  ax::AccessibilityBridge bridge((void*)CFBridgingRetain(engine));
  // Initialize ax node data.
  ax::AXNodeData ax_node_data;
  ax_node_data.role = ax::Role::kStaticText;
  ax_node_data.id = 0;
  ax_node_data.SetName("accessibility");
  // Initialize ax node.
  ax::AXNode ax_node(bridge.GetAXTree(), 0, -1, -1);
  ax_node.SetData(ax_node_data);
  // Initialize flutter accessibility mac.
  accessibility->Init(&bridge, &ax_node);
  // Verify the accessibility attribute matches.
  NSAccessibilityElement* native_accessibility = accessibility->GetNativeViewAccessible();
  std::string value = [native_accessibility.accessibilityValue UTF8String];
  EXPECT_TRUE(value == "accessibility");
  EXPECT_EQ(native_accessibility.accessibilityRole, NSAccessibilityStaticTextRole);
  EXPECT_EQ([native_accessibility.accessibilityChildren count], 0u);
  [engine shutDownEngine];
}

TEST(FlutterAccessibilityMac, CanPerformAction) {
  FlutterEngine* engine = CreateTestEngine();
  ax::FlutterAccessibilityMac* accessibility =
      static_cast<ax::FlutterAccessibilityMac*>(ax::FlutterAccessibility::Create());
  ax::AccessibilityBridge bridge((void*)CFBridgingRetain(engine));
  // Initialize ax node data.
  ax::AXNodeData ax_node_data;
  ax_node_data.role = ax::Role::kStaticText;
  ax_node_data.id = 1;
  ax_node_data.SetName("accessibility");
  // Initialize ax node.
  ax::AXNode ax_node(bridge.GetAXTree(), 0, -1, -1);
  ax_node.SetData(ax_node_data);
  // Initialize flutter accessibility mac.
  accessibility->Init(&bridge, &ax_node);

  // Set up embedder API mock.
  FlutterSemanticsAction called_action;
  uint64_t called_id;

  engine.embedderAPI.DispatchSemanticsAction = MOCK_ENGINE_PROC(
      DispatchSemanticsAction,
      ([&called_id, &called_action](auto engine, uint64_t id, FlutterSemanticsAction action,
                                    const uint8_t* data, size_t data_length) {
        called_id = id;
        called_action = action;
        return kSuccess;
      }));

  // Performs an AXAction.
  ax::AXActionData action_data;
  action_data.action = ax::Action::kDoDefault;
  accessibility->AccessibilityPerformAction(action_data);

  EXPECT_EQ(called_action, FlutterSemanticsAction::kFlutterSemanticsActionTap);
  EXPECT_EQ(called_id, 1u);
  [engine shutDownEngine];
}

}  // flutter::testing
