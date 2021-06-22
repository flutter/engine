// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ui/testing/views/coordinate_test_view.h"

namespace scenic {

CoordinateTestView::CoordinateTestView(ViewContext context, const std::string& debug_name)
    : BackgroundView(std::move(context), debug_name) {}

void CoordinateTestView::Draw(float cx, float cy, float sx, float sy) {
  BackgroundView::Draw(cx, cy, sx, sy);

  EntityNode root_node(session());
  view()->AddChild(root_node);

  static const float pane_width = sx / 2;
  static const float pane_height = sy / 2;

  for (uint8_t i = 0; i < 2; i++) {
    for (uint8_t j = 0; j < 2; j++) {
      Rectangle pane_shape(session(), pane_width, pane_height);
      Material pane_material(session());
      pane_material.SetColor(i * 255, 0, j * 255, 255);

      ShapeNode pane_node(session());
      pane_node.SetShape(pane_shape);
      pane_node.SetMaterial(pane_material);
      pane_node.SetTranslation((i + 0.5f) * pane_width, (j + 0.5f) * pane_height, -20);
      root_node.AddChild(pane_node);
    }
  }

  Rectangle pane_shape(session(), sx / 4, sy / 4);
  Material pane_material(session());
  pane_material.SetColor(0, 255, 0, 255);

  ShapeNode pane_node(session());
  pane_node.SetShape(pane_shape);
  pane_node.SetMaterial(pane_material);
  pane_node.SetTranslation(0.5f * sx, 0.5f * sy, -40);
  root_node.AddChild(pane_node);
}

}  // namespace scenic
