// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/color_source_text_contents.h"
#include <iostream>
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/texture_contents.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

ColorSourceTextContents::ColorSourceTextContents() = default;

ColorSourceTextContents::~ColorSourceTextContents() = default;

void ColorSourceTextContents::SetTextContents(
    std::shared_ptr<TextContents> text_contents) {
  text_contents_ = text_contents;
}

void ColorSourceTextContents::SetColorSourceContents(
    std::shared_ptr<ColorSourceContents> color_source_contents) {
  color_source_contents_ = color_source_contents;
}

std::optional<Rect> ColorSourceTextContents::GetCoverage(
    const Entity& entity) const {
  return text_contents_->GetCoverage(entity);
}

void ColorSourceTextContents::SetTextPosition(Point position) {
  position_ = position;
}

bool ColorSourceTextContents::Render(const ContentContext& renderer,
                                     const Entity& entity,
                                     RenderPass& pass) const {
  auto coverage = text_contents_->GetCoverage(entity);
  if (!coverage.has_value()) {
    return true;
  }

  std::cerr << "Coverage Origin: " << coverage->origin << std::endl;
  std::cerr << "Text Position: " << position_ << std::endl;
  // auto origin = coverage->origin;
  // auto effect_transform = Matrix::MakeTranslation(Vector3(-origin.x,
  // -origin.y, 0)); text_contents_->SetInverseMatrix(effect_transform);
  text_contents_->SetColor(Color::Black());
  color_source_contents_->SetGeometry(
      Geometry::MakeRect(Rect::MakeSize(coverage->size)));

  auto new_texture = renderer.MakeSubpass(
      "Text Color Blending", ISize::Ceil(coverage.value().size),
      [&](const ContentContext& context, RenderPass& pass) {
        Entity entity;
        entity.SetContents(text_contents_);
        entity.SetBlendMode(BlendMode::kSource);
        if (!entity.Render(context, pass)) {
          return false;
        }

        entity.SetContents(color_source_contents_);
        entity.SetBlendMode(BlendMode::kSourceIn);
        return entity.Render(context, pass);
      });
  if (!new_texture) {
    return false;
  }
  auto texture_contents = TextureContents::MakeRect(coverage.value());
  texture_contents->SetTexture(new_texture);
  texture_contents->SetSourceRect(Rect::MakeSize(new_texture->GetSize()));
  return texture_contents->Render(renderer, entity, pass);
}

}  // namespace impeller
