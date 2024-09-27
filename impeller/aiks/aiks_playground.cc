// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/aiks_playground.h"

#include <memory>

#include "impeller/aiks/aiks_context.h"
#include "impeller/display_list/dl_dispatcher.h"
#include "impeller/typographer/backends/skia/typographer_context_skia.h"
#include "impeller/typographer/typographer_context.h"

namespace impeller {

AiksPlayground::AiksPlayground()
    : typographer_context_(TypographerContextSkia::Make()) {}

AiksPlayground::~AiksPlayground() = default;

void AiksPlayground::SetTypographerContext(
    std::shared_ptr<TypographerContext> typographer_context) {
  typographer_context_ = std::move(typographer_context);
}

void AiksPlayground::TearDown() {
  PlaygroundTest::TearDown();
}

bool AiksPlayground::ImGuiBegin(const char* name,
                                bool* p_open,
                                ImGuiWindowFlags flags) {
  ImGui::Begin(name, p_open, flags);
  return true;
}

bool AiksPlayground::OpenPlaygroundHere(
    const sk_sp<flutter::DisplayList>& list) {
  return OpenPlaygroundHere([list]() { return list; });
}

bool AiksPlayground::OpenPlaygroundHere(
    const AiksDlPlaygroundCallback& callback) {
  AiksContext renderer(GetContext(), typographer_context_);

  if (!renderer.IsValid()) {
    return false;
  }

  return Playground::OpenPlaygroundHere(
      [&renderer, &callback](RenderTarget& render_target) -> bool {
        auto display_list = callback();
        TextFrameDispatcher collector(renderer.GetContentContext(),  //
                                      Matrix(),                      //
                                      Rect::MakeMaximum()            //
        );
        display_list->Dispatch(collector);

        CanvasDlDispatcher impeller_dispatcher(
            renderer.GetContentContext(), render_target,
            display_list->root_has_backdrop_filter(),
            display_list->max_root_blend_mode(), IRect::MakeMaximum());
        display_list->Dispatch(impeller_dispatcher);
        impeller_dispatcher.FinishRecording();
        renderer.GetContentContext().GetTransientsBuffer().Reset();
        renderer.GetContentContext().GetLazyGlyphAtlas()->ResetTextFrames();
        return true;
      });
}

}  // namespace impeller
