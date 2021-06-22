// Copyright 2019 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/ui/testing/views/rotated_square_view.h"

namespace scenic {

RotatedSquareView::RotatedSquareView(ViewContext context, const std::string& debug_name)
    : BackgroundView(std::move(context), debug_name), square_node_(session()) {
  Material square_material(session());
  square_material.SetColor(0xf5, 0x00, 0x57, 0xff);  // Pink A400
  square_node_.SetMaterial(square_material);
  view()->AddChild(square_node_);
}

void RotatedSquareView::Draw(float cx, float cy, float sx, float sy) {
  BackgroundView::Draw(cx, cy, sx, sy);

  const float square_size = std::min(sx, sy) * .6f;

  Rectangle square_shape(session(), square_size, square_size);
  square_node_.SetShape(square_shape);
  square_node_.SetTranslation({cx, cy, -kSquareElevation});
  square_node_.SetRotation({0.f, 0.f, sinf(kSquareAngle * .5f), cosf(kSquareAngle * .5f)});
}

}  // namespace scenic
