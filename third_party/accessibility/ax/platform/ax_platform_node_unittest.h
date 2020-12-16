// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCESSIBILITY_AX_PLATFORM_AX_PLATFORM_NODE_UNITTEST_H_
#define ACCESSIBILITY_AX_PLATFORM_AX_PLATFORM_NODE_UNITTEST_H_

#include "gtest/gtest.h"

#include "ax/ax_enums.h"
#include "ax/ax_node.h"
#include "ax/ax_node_data.h"
#include "ax/ax_tree_id.h"
#include "ax/ax_tree_update.h"
#include "ax/test_ax_tree_manager.h"

namespace ax {

class AXPlatformNodeTest : public testing::Test, public TestAXTreeManager {
 public:
  AXPlatformNodeTest();
  ~AXPlatformNodeTest() override;
  AXPlatformNodeTest(const AXPlatformNodeTest&) = delete;
  AXPlatformNodeTest& operator=(const AXPlatformNodeTest&) = delete;

 protected:
  // Initialize given an AXTreeUpdate.
  void Init(const AXTreeUpdate& initial_state);

  // Convenience functions to initialize directly from a few AXNodeData objects.
  void Init(const ax::AXNodeData& node1,
            const ax::AXNodeData& node2 = ax::AXNodeData(),
            const ax::AXNodeData& node3 = ax::AXNodeData(),
            const ax::AXNodeData& node4 = ax::AXNodeData(),
            const ax::AXNodeData& node5 = ax::AXNodeData(),
            const ax::AXNodeData& node6 = ax::AXNodeData(),
            const ax::AXNodeData& node7 = ax::AXNodeData(),
            const ax::AXNodeData& node8 = ax::AXNodeData(),
            const ax::AXNodeData& node9 = ax::AXNodeData(),
            const ax::AXNodeData& node10 = ax::AXNodeData(),
            const ax::AXNodeData& node11 = ax::AXNodeData(),
            const ax::AXNodeData& node12 = ax::AXNodeData());

  AXTreeUpdate BuildTextField();
  AXTreeUpdate BuildTextFieldWithSelectionRange(int32_t start, int32_t stop);
  AXTreeUpdate BuildContentEditable();
  AXTreeUpdate BuildContentEditableWithSelectionRange(int32_t start,
                                                      int32_t end);
  AXTreeUpdate Build3X3Table();
  AXTreeUpdate BuildAriaColumnAndRowCountGrids();

  AXTreeUpdate BuildListBox(bool option_1_is_selected,
                            bool option_2_is_selected,
                            bool option_3_is_selected,
                            const std::vector<ax::State>& additional_state);
};

}  // namespace ax

#endif  // ACCESSIBILITY_AX_PLATFORM_AX_PLATFORM_NODE_UNITTEST_H_
