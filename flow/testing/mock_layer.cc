// Copyright 2019 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/testing/mock_layer.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

MockLayer::MockLayer(SkPath path,
                     SkPaint paint,
                     bool fake_has_platform_view,
                     bool fake_needs_system_composite)
    : fake_paint_path_(path),
      fake_paint_(paint),
      fake_has_platform_view_(fake_has_platform_view),
      fake_needs_system_composite_(fake_needs_system_composite) {}

void MockLayer::Preroll(PrerollContext* context, const SkMatrix& matrix) {
  parent_mutators_ = context->mutators_stack;
  parent_matrix_ = matrix;
  parent_cull_rect_ = context->cull_rect;
  parent_elevation_ = context->total_elevation;
  parent_has_platform_view_ = context->has_platform_view;

  SkRect total_paint_bounds = fake_paint_path_.getBounds();
  bool child_has_platform_view = fake_has_platform_view_;
  bool child_needs_system_composite = fake_needs_system_composite_;
  if (child_layer_) {
    child_layer_->Preroll(context, matrix);
    child_has_platform_view =
        child_has_platform_view || context->has_platform_view;
    child_needs_system_composite =
        child_needs_system_composite || child_layer_->needs_system_composite();
    total_paint_bounds.join(child_layer_->paint_bounds());
  }
  context->has_platform_view = child_has_platform_view;
  set_paint_bounds(total_paint_bounds);
  set_needs_system_composite(child_needs_system_composite);
}

void MockLayer::Paint(PaintContext& context) const {
  FML_DCHECK(needs_painting());

  context.leaf_nodes_canvas->drawPath(fake_paint_path_, fake_paint_);
  if (child_layer_) {
    child_layer_->Paint(context);
  }
}

void MockLayer::ExpectMutators(const std::vector<Mutator>& stack) {
  uint elements_count = 0;
  for (auto mutator_iter = parent_mutators_.Bottom();
       mutator_iter != parent_mutators_.Top(); mutator_iter++) {
    EXPECT_LT(elements_count, stack.size());
    EXPECT_NE(*mutator_iter, nullptr);
    EXPECT_EQ(**mutator_iter, stack[elements_count]);
    elements_count++;
  }
  EXPECT_EQ(elements_count, stack.size());
}

void MockLayer::ExpectParentMatrix(const SkMatrix& matrix) {
  EXPECT_EQ(matrix, parent_matrix_);
}

void MockLayer::ExpectParentCullRect(const SkRect& rect) {
  EXPECT_EQ(rect, parent_cull_rect_);
}

void MockLayer::ExpectParentElevation(float elevation) {
  EXPECT_EQ(elevation, parent_elevation_);
}

void MockLayer::ExpectParentHasPlatformView(bool has_platform_view) {
  EXPECT_EQ(has_platform_view, parent_has_platform_view_);
}

}  // namespace testing
}  // namespace flutter
