// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/impeller/renderer/texture.h"
#include "impeller/entity/contents/color_source_contents.h"
#include "impeller/entity/entity.h"

namespace impeller {

class FramebufferBlendContents final : public ColorSourceContents {
 public:
  FramebufferBlendContents();

  ~FramebufferBlendContents() override;

  void SetBlendMode(BlendMode blend_mode);

  void SetForegroundColor(Color color);

  void SetChildContents(std::shared_ptr<Contents> child_contents);

  // |Contents|
  bool Render(const ContentContext& renderer,
              const Entity& entity,
              RenderPass& pass) const override;

 private:
  BlendMode blend_mode_;
  std::optional<Color> foreground_color_ = std::nullopt;
  std::optional<std::shared_ptr<Contents>> child_contents_;

  FML_DISALLOW_COPY_AND_ASSIGN(FramebufferBlendContents);
};

}  // namespace impeller
