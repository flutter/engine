// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_PLAYGROUND_WIDGETS_H_
#define FLUTTER_IMPELLER_PLAYGROUND_WIDGETS_H_

#include <tuple>

#include "impeller/base/strings.h"
#include "impeller/geometry/color.h"
#include "impeller/geometry/point.h"
#include "third_party/imgui/imgui.h"

#define IMPELLER_PLAYGROUND_POINT(default_position, radius, color)             \
  ({                                                                           \
    static impeller::Point position = default_position;                        \
    static impeller::Point reset_position = default_position;                  \
    float radius_ = radius;                                                    \
    impeller::Color color_ = color;                                            \
                                                                               \
    static bool dragging = false;                                              \
    impeller::Point mouse_pos(ImGui::GetMousePos().x, ImGui::GetMousePos().y); \
    static impeller::Point prev_mouse_pos = mouse_pos;                         \
                                                                               \
    if (ImGui::IsKeyPressed(ImGuiKey_R)) {                                     \
      position = reset_position;                                               \
      dragging = false;                                                        \
    }                                                                          \
                                                                               \
    bool hovering = position.GetDistance(mouse_pos) < radius_ &&               \
                    position.GetDistance(prev_mouse_pos) < radius_;            \
    if (!ImGui::IsMouseDown(0)) {                                              \
      dragging = false;                                                        \
    } else if (hovering && ImGui::IsMouseClicked(0)) {                         \
      dragging = true;                                                         \
    }                                                                          \
    if (dragging) {                                                            \
      position += mouse_pos - prev_mouse_pos;                                  \
    }                                                                          \
    ImGui::GetBackgroundDrawList()->AddCircleFilled(                           \
        {position.x, position.y}, radius_,                                     \
        ImColor(color_.red, color_.green, color_.blue,                         \
                (hovering || dragging) ? 0.6f : 0.3f));                        \
    if (hovering || dragging) {                                                \
      ImGui::GetBackgroundDrawList()->AddText(                                 \
          {position.x - radius_, position.y + radius_ + 10},                   \
          ImColor(color_.red, color.green, color.blue, 1.0f),                  \
          impeller::SPrintF("x:%0.3f y:%0.3f", position.x, position.y)         \
              .c_str());                                                       \
    }                                                                          \
    prev_mouse_pos = mouse_pos;                                                \
    position;                                                                  \
  })

namespace impeller {
std::tuple<Point, Point> DrawPlaygroundLine(Point default_position_a,
                                            Point default_position_b,
                                            Scalar radius,
                                            Color color_a,
                                            Color color_b);
}  // namespace impeller
#endif  // FLUTTER_IMPELLER_PLAYGROUND_WIDGETS_H_
