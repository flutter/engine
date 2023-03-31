// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"
#include "impeller/aiks/aiks_context.h"
#include "impeller/aiks/picture.h"
#include "impeller/playground/playground_test.h"

namespace impeller {

class AiksPlayground : public PlaygroundTest {
 public:
  using AiksPlaygroundCallback =
      std::function<bool(AiksContext& renderer, RenderTarget& render_target)>;

  using PictureCallback =
      std::function<std::optional<Picture>(AiksContext& renderer)>;

  AiksPlayground();

  ~AiksPlayground();

  bool OpenPlaygroundHere(const Picture& picture);

  /// Deprecated, use PictureCallback variant.
  bool OpenPlaygroundHere(AiksPlaygroundCallback callback);

  /// Opens an interactive playground window. All calls to imgui should happen
  /// in `update_imgui`.
  bool OpenPlaygroundHere(std::function<void()> update_imgui,
                          PictureCallback callback);

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(AiksPlayground);
};

}  // namespace impeller
