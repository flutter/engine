// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/aiks_playground.h"

#include "impeller/aiks/aiks_context.h"

#include "third_party/imgui/imgui.h"

namespace impeller {

AiksPlayground::AiksPlayground() = default;

AiksPlayground::~AiksPlayground() = default;

bool AiksPlayground::OpenPlaygroundHere(const Picture& picture) {
  return OpenPlaygroundHere(
      [&picture](AiksContext& renderer, RenderTarget& render_target) -> bool {
        return renderer.Render(picture, render_target);
      });
}

bool AiksPlayground::OpenPlaygroundHere(AiksPlaygroundCallback callback) {
  if (!switches_.enable_playground) {
    return true;
  }

  AiksContext renderer(GetContext());

  if (!renderer.IsValid()) {
    return false;
  }

  return Playground::OpenPlaygroundHere(
      [&renderer, &callback](RenderTarget& render_target) -> bool {
        static bool wireframe = false;
        if (ImGui::IsKeyPressed(ImGuiKey_Z)) {
          wireframe = !wireframe;
          renderer.GetContentContext().SetWireframe(wireframe);
        }
        return callback(renderer, render_target);
      });
}

bool AiksPlayground::OpenPlaygroundHere(
    void* state,
    const std::function<void(void*)>& update_imgui,
    const PictureCallback& callback) {
  if (!switches_.enable_playground) {
    return true;
  }

  AiksContext renderer(GetContext());

  if (!renderer.IsValid()) {
    return false;
  }

  return Playground::OpenPlaygroundHere(
      [&renderer, &callback, update_imgui,
       state](RenderTarget& render_target) -> bool {
        static bool wireframe = false;
        if (ImGui::IsKeyPressed(ImGuiKey_Z)) {
          wireframe = !wireframe;
          renderer.GetContentContext().SetWireframe(wireframe);
        }

        update_imgui(state);

        std::optional<Picture> picture = callback(state, renderer);
        if (!picture.has_value()) {
          return false;
        }

        return renderer.Render(*picture, render_target);
      });
}

}  // namespace impeller
