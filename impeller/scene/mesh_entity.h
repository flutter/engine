// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/macros.h"

#include "impeller/scene/scene_entity.h"

namespace impeller {
namespace scene {

class MeshEntity : SceneEntity {
 public:
  void Add(std::shared_ptr<SceneEntity> child);

 private:
  bool OnRender(SceneEncoder& encoder, const Camera& camera) const override;

  FML_DISALLOW_COPY_AND_ASSIGN(MeshEntity);
};

}  // namespace scene
}  // namespace impeller