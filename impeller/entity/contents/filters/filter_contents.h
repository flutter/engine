// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <variant>
#include <vector>

#include "impeller/entity/entity.h"
#include "impeller/renderer/formats.h"

namespace impeller {

class Pipeline;

class FilterContents : public Contents {
 public:
  using InputVariant =
      std::variant<std::shared_ptr<Texture>, std::shared_ptr<Contents>>;
  using InputTextures = std::vector<InputVariant>;

  static std::shared_ptr<FilterContents> MakeBlend(
      Entity::BlendMode blend_mode,
      InputTextures input_textures);

  static std::shared_ptr<FilterContents> MakeDirectionalGaussianBlur(
      InputVariant input_texture,
      Vector2 blur_vector);

  static std::shared_ptr<FilterContents>
  MakeGaussianBlur(InputVariant input_texture, Scalar sigma_x, Scalar sigma_y);

  static Rect GetBoundsForInput(const Entity& entity,
                                const InputVariant& input);

  FilterContents();

  ~FilterContents() override;

  /// @brief The input texture sources for this filter. All texture sources are
  ///        expected to have or produce premultiplied alpha colors.
  ///        Any input can either be a `Texture` or another `FilterContents`.
  ///
  ///        The number of required or optional textures depends on the
  ///        particular filter's implementation.
  void SetInputTextures(InputTextures input_textures);

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

  // |Contents|
  Rect GetBounds(const Entity& entity) const override;

  // |Contents|
  virtual std::optional<Snapshot> RenderToTexture(
      const ContentContext& renderer,
      const Entity& entity) const override;

 private:
  /// @brief Takes a set of zero or more input textures and writes to an output
  ///        texture.
  virtual bool RenderFilter(const std::vector<Snapshot>& input_textures,
                            const ContentContext& renderer,
                            RenderPass& pass,
                            const Matrix& transform) const = 0;

  InputTextures input_textures_;
  Rect destination_;

  FML_DISALLOW_COPY_AND_ASSIGN(FilterContents);
};

}  // namespace impeller
