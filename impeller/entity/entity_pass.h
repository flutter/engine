// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <vector>

#include "flutter/fml/macros.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/entity/contents/filters/filter_contents.h"
#include "impeller/entity/entity.h"
#include "impeller/entity/entity_pass_delegate.h"
#include "impeller/entity/inline_pass_context.h"
#include "impeller/renderer/render_target.h"
#include "impeller/typographer/lazy_glyph_atlas.h"

namespace impeller {

class ContentContext;

class EntityPass {
 public:
  using Element = std::variant<Entity, std::unique_ptr<EntityPass>>;
  using BackdropFilterProc =
      std::function<std::shared_ptr<FilterContents>(FilterInput::Ref)>;

  EntityPass();

  ~EntityPass();

  void SetDelegate(std::unique_ptr<EntityPassDelegate> delgate);

  size_t GetSubpassesDepth() const;

  std::unique_ptr<EntityPass> Clone() const;

  void AddEntity(Entity entity);

  void SetElements(std::vector<Element> elements);

  const std::shared_ptr<LazyGlyphAtlas>& GetLazyGlyphAtlas() const;

  EntityPass* AddSubpass(std::unique_ptr<EntityPass> pass);

  EntityPass* GetSuperpass() const;

  bool Render(ContentContext& renderer, RenderTarget render_target) const;

  void IterateAllEntities(std::function<bool(Entity&)> iterator);

  void SetTransformation(Matrix xformation);

  void SetStencilDepth(size_t stencil_depth);

  void SetBlendMode(Entity::BlendMode blend_mode);

  void SetBackdropFilter(std::optional<BackdropFilterProc> proc);

  std::optional<Rect> GetSubpassCoverage(const EntityPass& subpass) const;

  std::optional<Rect> GetElementsCoverage() const;

 private:
  struct EntityResult {
    /// @brief  The resulting entity that should be rendered. If `std::nullopt`,
    ///         there is nothing to render.
    std::optional<Entity> entity = std::nullopt;
    /// @brief  This is set to `false` if there was an unexpected rendering
    ///         error while resolving the Entity.
    bool success = false;

    static EntityResult Success(Entity e) { return {e, true}; }
    static EntityResult Failure() { return {std::nullopt, false}; }
    static EntityResult Empty() { return {std::nullopt, true}; }
  };

  EntityResult GetElementEntity(const EntityPass::Element& element,
                                ContentContext& renderer,
                                InlinePassContext& pass_context,
                                Point position,
                                uint32_t pass_depth,
                                size_t stencil_depth_floor) const;

  bool OnRender(ContentContext& renderer,
                RenderTarget render_target,
                Point position,
                uint32_t pass_depth,
                size_t stencil_depth_floor = 0,
                std::shared_ptr<Texture> backdrop_texture = nullptr) const;

  std::vector<Element> elements_;

  EntityPass* superpass_ = nullptr;
  Matrix xformation_;
  size_t stencil_depth_ = 0u;
  Entity::BlendMode blend_mode_ = Entity::BlendMode::kSourceOver;
  bool contains_advanced_blends_ = false;

  std::optional<BackdropFilterProc> backdrop_filter_proc_ = std::nullopt;

  std::unique_ptr<EntityPassDelegate> delegate_ =
      EntityPassDelegate::MakeDefault();
  std::shared_ptr<LazyGlyphAtlas> lazy_glyph_atlas_ =
      std::make_shared<LazyGlyphAtlas>();

  FML_DISALLOW_COPY_AND_ASSIGN(EntityPass);
};

struct CanvasStackEntry {
  Matrix xformation;
  size_t stencil_depth = 0u;
  bool is_subpass = false;
  bool contains_clips = false;
};

}  // namespace impeller
