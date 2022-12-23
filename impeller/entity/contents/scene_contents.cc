// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/contents/scene_contents.h"

#include "impeller/entity/contents/content_context.h"
#include "impeller/entity/contents/tiled_texture_contents.h"
#include "impeller/entity/entity.h"
#include "impeller/geometry/path_builder.h"
#include "impeller/renderer/formats.h"
#include "impeller/scene/camera.h"
#include "impeller/scene/scene.h"

namespace impeller {

SceneContents::SceneContents() = default;

SceneContents::~SceneContents() = default;

void SceneContents::SetCameraTransform(Matrix matrix) {
  camera_transform_ = matrix;
}

void SceneContents::SetNode(std::shared_ptr<scene::Node> node) {
  node_ = std::move(node);
}

bool SceneContents::Render(const ContentContext& renderer,
                           const Entity& entity,
                           RenderPass& pass) const {
  if (!node_) {
    return true;
  }
  auto coverage = GetCoverage(entity);
  if (!coverage.has_value()) {
    return true;
  }

  RenderTarget subpass_target = RenderTarget::CreateOffscreenMSAA(
      *renderer.GetContext(),            // context
      ISize(coverage.value().size),      // size
      "SceneContents",                   // label
      StorageMode::kDeviceTransient,     // color_storage_mode
      StorageMode::kDevicePrivate,       // color_resolve_storage_mode
      LoadAction::kDontCare,             // color_load_action
      StoreAction::kMultisampleResolve,  // color_store_action
      StorageMode::kDeviceTransient,     // stencil_storage_mode
      LoadAction::kDontCare,             // stencil_load_action
      StoreAction::kDontCare             // stencil_store_action
  );

  scene::Scene scene(renderer.GetSceneContext());
  scene.GetRoot().AddChild(node_);

  scene.Render(pass.GetRenderTarget(), camera_transform_);

  // Render the texture to the pass.
  TiledTextureContents final;
  final.SetGeometry(GetGeometry());
  final.SetTexture(subpass_target.GetRenderTargetTexture());
  return final.Render(renderer, entity, pass);
}

}  // namespace impeller
