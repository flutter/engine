// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_WASM_IMPELLER_SCENE_H_
#define FLUTTER_IMPELLER_TOOLKIT_WASM_IMPELLER_SCENE_H_

#include <array>

#include "flutter/fml/time/time_point.h"
#include "impeller/core/texture.h"
#include "impeller/renderer/render_pass.h"
#include "impeller/toolkit/wasm/scene.h"

namespace impeller::wasm {

class ImpellerScene final : public Scene {
 public:
  ImpellerScene();

  ~ImpellerScene() override;

  ImpellerScene(const ImpellerScene&) = delete;

  ImpellerScene& operator=(const ImpellerScene&) = delete;

  // |Scene|
  bool Setup(const Context& context) override;

  // |Scene|
  bool Render(const Context& context, RenderPass& pass) override;

  // |Scene|
  bool Teardown(const Context& context) override;

 private:
  std::shared_ptr<Pipeline<PipelineDescriptor>> pipeline_;
  fml::TimePoint start_time_;
  std::shared_ptr<Texture> blue_noise_;
  std::shared_ptr<Texture> cube_map_;

  Scalar GetSecondsElapsed() const;

  std::shared_ptr<Texture> CreateTextureForFixture(const char* fixture) const;

  std::shared_ptr<Texture> CreateTextureCubeForFixture(
      std::array<const char*, 6> fixture_names) const;
};

}  // namespace impeller::wasm

#endif  // FLUTTER_IMPELLER_TOOLKIT_WASM_IMPELLER_SCENE_H_
