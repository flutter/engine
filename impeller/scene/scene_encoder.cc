// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/macros.h"

#include "impeller/scene/scene_encoder.h"

namespace impeller {
namespace scene {

SceneEncoder::SceneEncoder() = default;

std::shared_ptr<CommandBuffer> SceneEncoder::BuildSceneCommandBuffer() const {

}

}
}  // namespace impeller