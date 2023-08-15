// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <variant>
#include <vector>

#include "flutter/fml/macros.h"
#include "impeller/entity/contents/contents.h"
#include "impeller/geometry/color.h"
#include "impeller/typographer/glyph_atlas.h"
#include "impeller/typographer/text_frame.h"

namespace impeller {

class LazyGlyphAtlas;
class Context;

class TextContents final : public Contents {
 public:
  struct TextFrameInfo {
    TextFrame frame;
  };

  TextContents();

  ~TextContents();

  void AddTextFrame(const TextFrame& frame);

  const std::vector<TextFrameInfo>& GetTextFrames() { return frames_; }

  void SetColor(Color color);

  Color GetColor() const;

  // |Contents|
  bool CanInheritOpacity(const Entity& entity) const override;

  // |Contents|
  void SetInheritedOpacity(Scalar opacity) override;

  void SetOffset(Vector2 offset);

  Vector2 GetOffset() const { return offset_; }

  std::optional<Rect> GetTextFrameBounds() const;

  // |Contents|
  std::optional<Rect> GetCoverage(const Entity& entity) const override;

  // |Contents|
  void PopulateGlyphAtlas(
      const std::shared_ptr<LazyGlyphAtlas>& lazy_glyph_atlas,
      Scalar scale) override;

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

  /// Get the position that the text will be drawn to relative to its transform.
  ///
  /// This value is baked into the entity's transform but is useful for
  /// calculating offsets for batching text draws.
  const Point& GetPosition() const { return position_; }

  void SetPosition(const Point& point) { position_ = point; }

 private:
  std::vector<TextFrameInfo> frames_;
  Scalar scale_ = 1.0;
  Color color_;
  Scalar inherited_opacity_ = 1.0;
  Vector2 offset_;
  Point position_;

  std::shared_ptr<GlyphAtlas> ResolveAtlas(
      GlyphAtlas::Type type,
      const std::shared_ptr<LazyGlyphAtlas>& lazy_atlas,
      std::shared_ptr<Context> context) const;

  FML_DISALLOW_COPY_AND_ASSIGN(TextContents);
};

}  // namespace impeller
