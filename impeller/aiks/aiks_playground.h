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

  template <typename T>
  using PictureCallback =
      std::function<std::optional<Picture>(T* state, AiksContext& renderer)>;

  template <typename T>
  using UpdateCallback = std::function<void(T* state)>;

  AiksPlayground();

  ~AiksPlayground();

  bool OpenPlaygroundHere(const Picture& picture);

  /// Deprecated, use PictureCallback variant.
  bool OpenPlaygroundHere(AiksPlaygroundCallback callback);

  /// Opens an interactive playground window. All calls to imgui should happen
  /// in `update_imgui`.
  template <typename T>
  bool OpenPlaygroundHere(T* state,
                          const UpdateCallback<T>& update_imgui,
                          const PictureCallback<T>& callback) {
    return OpenPlaygroundHereImpl(
        state,
        [update_imgui](void* state) {
          T* t_state = static_cast<T*>(state);
          update_imgui(t_state);
        },
        [callback](void* state, AiksContext& renderer) {
          T* t_state = static_cast<T*>(state);
          return callback(t_state, renderer);
        });
  }

 private:
  bool OpenPlaygroundHereImpl(void* state,
                              const UpdateCallback<void>& update_imgui,
                              const PictureCallback<void>& callback);

  FML_DISALLOW_COPY_AND_ASSIGN(AiksPlayground);
};

}  // namespace impeller
