// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/aiks_playground.h"

#include "impeller/aiks/aiks_context.h"

#include "impeller/typographer/backends/skia/text_render_context_skia.h"
#include "third_party/imgui/imgui.h"

namespace impeller {

AiksPlayground::AiksPlayground() = default;

AiksPlayground::~AiksPlayground() = default;

bool AiksPlayground::OpenPlaygroundHere(
    const Picture& picture,
    std::unique_ptr<TextRenderContext> text_render_context_override) {
  auto text_context = text_render_context_override
                          ? std::move(text_render_context_override)
                          : TextRenderContextSkia::Make();
  return OpenPlaygroundHere(
      [&picture](AiksContext& renderer, RenderTarget& render_target) -> bool {
        return renderer.Render(picture, render_target);
      },
      std::move(text_context));
}

bool AiksPlayground::OpenPlaygroundHere(
    AiksPlaygroundCallback callback,
    std::unique_ptr<TextRenderContext> text_render_context_override) {
  if (!switches_.enable_playground) {
    return true;
  }

  auto text_context = text_render_context_override
                          ? std::move(text_render_context_override)
                          : TextRenderContextSkia::Make();
  AiksContext renderer(GetContext(), std::move(text_context));

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

}  // namespace impeller
