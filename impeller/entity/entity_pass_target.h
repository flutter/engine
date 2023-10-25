// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "fml/macros.h"
#include "impeller/renderer/command.h"
#include "impeller/renderer/render_target.h"

namespace impeller {

class InlinePassContext;

struct ClipStateHacker {
  std::vector<Command> clip_affecting_commands;
};

class EntityPassTarget {
 public:
  explicit EntityPassTarget(const RenderTarget& render_target,
                            bool supports_read_from_resolve);

  /// @brief  Flips the backdrop and returns a readable texture that can be
  ///         bound/sampled to restore the previous pass.
  ///
  ///         After this method is called, a new `RenderPass` that attaches the
  ///         result of `GetRenderTarget` is guaranteed to be able to read the
  ///         previous pass's backdrop texture (which is returned by this
  ///         method).
  std::shared_ptr<Texture> Flip(Allocator& allocator);

  const RenderTarget& GetRenderTarget() const;

  bool IsValid() const;

  std::shared_ptr<ClipStateHacker> GetHacker() { return clip_state_hacker_; }

 private:
  RenderTarget target_;
  std::shared_ptr<Texture> secondary_color_texture_;
  std::shared_ptr<ClipStateHacker> clip_state_hacker_ =
      std::make_shared<ClipStateHacker>();

  bool supports_read_from_resolve_;

  friend InlinePassContext;

  FML_DISALLOW_ASSIGN(EntityPassTarget);
};

}  // namespace impeller
