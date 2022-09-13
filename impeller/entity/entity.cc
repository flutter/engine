// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/entity.h"

#include <algorithm>
#include <optional>

#include "impeller/base/validation.h"
#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/filters/filter_contents.h"
#include "impeller/entity/entity_pass.h"
#include "impeller/geometry/vector.h"
#include "impeller/renderer/render_pass.h"

namespace impeller {

Entity::Entity() = default;

Entity::~Entity() = default;

const Matrix& Entity::GetTransformation() const {
  return transformation_;
}

void Entity::SetTransformation(const Matrix& transformation) {
  transformation_ = transformation;
}

std::optional<Rect> Entity::GetCoverage() const {
  if (!contents_) {
    return std::nullopt;
  }

  return contents_->GetCoverage(*this);
}

Contents::StencilCoverage Entity::GetStencilCoverage(
    const std::optional<Rect>& current_stencil_coverage) const {
  if (!contents_) {
    return {};
  }
  return contents_->GetStencilCoverage(*this, current_stencil_coverage);
}

bool Entity::ShouldRender(const std::optional<Rect>& stencil_coverage) const {
  return contents_->ShouldRender(*this, stencil_coverage);
}

void Entity::SetContents(std::shared_ptr<Contents> contents) {
  contents_ = std::move(contents);
}

const std::shared_ptr<Contents>& Entity::GetContents() const {
  return contents_;
}

void Entity::SetStencilDepth(uint32_t depth) {
  stencil_depth_ = depth;
}

uint32_t Entity::GetStencilDepth() const {
  return stencil_depth_;
}

void Entity::IncrementStencilDepth(uint32_t increment) {
  stencil_depth_ += increment;
}

void Entity::SetBlendMode(BlendMode blend_mode) {
  blend_mode_ = blend_mode;
}

Entity::BlendMode Entity::GetBlendMode() const {
  return blend_mode_;
}

bool Entity::BlendModeShouldCoverWholeScreen(BlendMode blend_mode) {
  switch (blend_mode) {
    case BlendMode::kClear:
    case BlendMode::kSource:
    case BlendMode::kSourceIn:
    case BlendMode::kDestinationIn:
    case BlendMode::kSourceOut:
    case BlendMode::kDestinationOut:
    case BlendMode::kDestinationATop:
    case BlendMode::kXor:
    case BlendMode::kModulate:
      return true;
    default:
      return false;
  }
}

bool Entity::Render(const ContentContext& renderer,
                    RenderPass& parent_pass) const {
  if (!contents_) {
    return true;
  }

  return contents_->Render(renderer, *this, parent_pass);
}

Vector4 min(Vector4 value, float threshold) {
  return Vector4(std::min(value.x, threshold), std::min(value.y, threshold),
                 std::min(value.z, threshold), std::min(value.w, threshold));
}

Color blendColor(const Color& src,
                 const Color& dst,
                 Entity::BlendMode blend_mode) {
  switch (blend_mode) {
    case Entity::BlendMode::kClear:
      return Color::BlackTransparent();
    case Entity::BlendMode::kSource:
      return src;
    case Entity::BlendMode::kDestination:
      return dst;
    case Entity::BlendMode::kSourceOver:
      // r = s + (1-sa)*d
      return Color(Vector4(src) + Vector4(dst) * (1 - src.alpha));
    case Entity::BlendMode::kDestinationOver:
      // r = d + (1-da)*s
      return Color(Vector4(dst) + Vector4(src) * (1 - dst.alpha));
    case Entity::BlendMode::kSourceIn:
      // r = s * da
      return Color(Vector4(src) * dst.alpha);
    case Entity::BlendMode::kDestinationIn:
      // r = d * sa
      return Color(Vector4(dst) * src.alpha);
    case Entity::BlendMode::kSourceOut:
      // r = s * ( 1- da)
      return Color(Vector4(dst) * (1 - dst.alpha));
    case Entity::BlendMode::kDestinationOut:
      // r = d * (1-sa)
      return Color(Vector4(dst) * (1 - src.alpha));
    case Entity::BlendMode::kSourceATop:
      // r = s*da + d*(1-sa)
      return Color(Vector4(src) * dst.alpha + Vector4(dst) * (1 - src.alpha));
    case Entity::BlendMode::kDestinationATop:
      // r = d*sa + s*(1-da)
      return Color(Vector4(dst) * src.alpha + Vector4(src) * (1 - dst.alpha));
    case Entity::BlendMode::kXor:
      // r = s*(1-da) + d*(1-sa)
      return Color(Vector4(src) * (1 - dst.alpha) +
                   Vector4(dst) * (1 - src.alpha));
    case Entity::BlendMode::kPlus:
      // r = min(s + d, 1)
      return Color(min(Vector4(src) + Vector4(dst), 1));
    case Entity::BlendMode::kModulate:
      // r = s*d
      return src * dst;
    case Entity::BlendMode::kScreen: {
      // r = s + d - s*d
      auto v_s = Vector4(src);
      auto v_d = Vector4(src);
      return Color(v_s + v_d - (src * dst));
    }
    default:
      // TODO:
      return src;
  }
}
}  // namespace impeller
