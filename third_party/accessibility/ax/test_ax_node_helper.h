// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCESSIBILITY_AX_TEST_AX_NODE_HELPER_H_
#define ACCESSIBILITY_AX_TEST_AX_NODE_HELPER_H_

#include "ax_clipping_behavior.h"
#include "ax_coordinate_system.h"
#include "ax_node.h"
#include "ax_offscreen_result.h"
#include "ax_tree.h"

namespace ax {

// For testing, a TestAXNodeHelper wraps an AXNode. This is a simple
// version of TestAXNodeWrapper.
class TestAXNodeHelper {
 public:
  // Create TestAXNodeHelper instances on-demand from an AXTree and AXNode.
  static TestAXNodeHelper* GetOrCreate(AXTree* tree, AXNode* node);
  ~TestAXNodeHelper();

  SkRect GetBoundsRect(const AXCoordinateSystem coordinate_system,
                       const AXClippingBehavior clipping_behavior,
                       AXOffscreenResult* offscreen_result) const;
  SkRect GetInnerTextRangeBoundsRect(const int start_offset,
                                     const int end_offset,
                                     const AXCoordinateSystem coordinate_system,
                                     const AXClippingBehavior clipping_behavior,
                                     AXOffscreenResult* offscreen_result) const;

 private:
  TestAXNodeHelper(AXTree* tree, AXNode* node);
  int InternalChildCount() const;
  TestAXNodeHelper* InternalGetChild(int index) const;
  const AXNodeData& GetData() const;
  SkRect GetLocation() const;
  SkRect GetInlineTextRect(const int start_offset, const int end_offset) const;
  AXOffscreenResult DetermineOffscreenResult(SkRect bounds) const;

  AXTree* tree_;
  AXNode* node_;
};

}  // namespace ax

#endif  // ACCESSIBILITY_AX_TEST_AX_NODE_HELPER_H_
