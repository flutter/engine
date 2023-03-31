// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>

#include "flutter/fml/macros.h"
#include "flutter/impeller/aiks/aiks_context.h"
#include "flutter/impeller/playground/playground.h"
#include "flutter/impeller/renderer/render_target.h"
#include "flutter/testing/testing.h"

namespace impeller {

class GoldenPlaygroundTest
    : public ::testing::TestWithParam<PlaygroundBackend> {
 public:
  using AiksPlaygroundCallback =
      std::function<bool(AiksContext& renderer, RenderTarget& render_target)>;

  template <typename T>
  using PictureCallback =
      std::function<std::optional<Picture>(T* state, AiksContext& renderer)>;

  template <typename T>
  using UpdateCallback = std::function<void(T* state)>;

  GoldenPlaygroundTest();

  void SetUp();

  PlaygroundBackend GetBackend() const;

  bool OpenPlaygroundHere(const Picture& picture);

  /// Deprecated: Use the PictureCallback variant.
  bool OpenPlaygroundHere(const AiksPlaygroundCallback& callback);

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

  std::shared_ptr<Texture> CreateTextureForFixture(
      const char* fixture_name,
      bool enable_mipmapping = false) const;

  std::shared_ptr<Context> GetContext() const;

  Point GetContentScale() const;

  Scalar GetSecondsElapsed() const;

  ISize GetWindowSize() const;

 private:
  struct GoldenPlaygroundTestImpl;

  bool OpenPlaygroundHereImpl(void* state,
                              const UpdateCallback<void>& update_imgui,
                              const PictureCallback<void>& callback);

  // This is only a shared_ptr so it can work with a forward declared type.
  std::shared_ptr<GoldenPlaygroundTestImpl> pimpl_;
  FML_DISALLOW_COPY_AND_ASSIGN(GoldenPlaygroundTest);
};

}  // namespace impeller
