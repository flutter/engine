// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/aiks/picture.h"

#include <memory>
#include <optional>

#include "impeller/base/validation.h"
#include "impeller/entity/entity.h"
#include "impeller/renderer/render_target.h"
#include "impeller/renderer/snapshot.h"

namespace impeller {

std::optional<Snapshot> Picture::Snapshot(AiksContext& context) {
  auto coverage = pass_->GetElementsCoverage(std::nullopt);
  if (!coverage.has_value() || coverage->IsEmpty()) {
    return std::nullopt;
  }
  return DoSnapshot(context, coverage.value());
}

std::shared_ptr<Image> Picture::ToImage(AiksContext& context, ISize size) {
  if (size.IsEmpty()) {
    return nullptr;
  }
  auto snapshot = DoSnapshot(context, Rect::MakeSize(size));
  return snapshot.has_value() ? std::make_shared<Image>(snapshot->texture)
                              : nullptr;
}

std::optional<Snapshot> Picture::DoSnapshot(AiksContext& context, Rect rect) {
  FML_DCHECK(!rect.IsEmpty());

  const auto translate = Matrix::MakeTranslation(-rect.origin);
  pass_->IterateAllEntities([&translate](auto& entity) -> bool {
    entity.SetTransformation(translate * entity.GetTransformation());
    return true;
  });

  // This texture isn't host visible, but we might want to add host visible
  // features to Image someday.
  auto target = RenderTarget::CreateOffscreen(
      *context.GetContext(), ISize(rect.size.width, rect.size.height));
  if (!target.IsValid()) {
    VALIDATION_LOG << "Could not create valid RenderTarget.";
    return std::nullopt;
  }

  if (!context.Render(*this, target)) {
    VALIDATION_LOG << "Could not render Picture to Texture.";
    return std::nullopt;
  }

  auto texture = target.GetRenderTargetTexture();
  if (!texture) {
    VALIDATION_LOG << "RenderTarget has no target texture.";
    return std::nullopt;
  }

  return impeller::Snapshot{
      .texture = std::move(texture),
      .transform = translate.MakeTranslation(rect.origin)};
}

}  // namespace impeller
