// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/scene/scene.h"

#include <memory>
#include <utility>

#include "impeller/renderer/render_target.h"
#include "impeller/scene/scene_encoder.h"

namespace impeller {
namespace scene {

Scene::Scene(std::shared_ptr<Context> context) : context_(std::move(context)){};

void Scene::Add(std::shared_ptr<SceneEntity> child) {
  root_.Add(std::move(child));
}

bool Scene::Render(const RenderTarget& render_target,
                   const Camera& camera) const {
  SceneEncoder encoder;
  return root_.Render(encoder, camera);
  std::shared_ptr<CommandBuffer> geometry =
      encoder.BuildGeometryCommandBuffer(render_target);
}

}  // namespace scene
}  // namespace impeller