// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/shader_bundle/shader_bundle.h"

#include "impeller/shader_bundle/shader_bundle_flatbuffers.h"

namespace impeller {

ShaderBundle::ShaderBundle(std::shared_ptr<fml::Mapping> payload)
    : payload_(std::move(payload)) {
  if (payload_ == nullptr || !payload_->GetMapping()) {
    return;
  }
  if (!fb::ShaderBundleBufferHasIdentifier(payload_->GetMapping())) {
    return;
  }
  auto shader_bundle = fb::GetShaderBundle(payload_->GetMapping());
  if (!shader_bundle) {
    return;
  }

  auto* shaders = shader_bundle->shaders();
  for (size_t i = 0; i < shaders->size(); i++) {
    const fb::Shader* shader = shaders->Get(i);
    shaders_[shader->name()->str()] = Shader(shader);
    if (!shaders_[shader->name()->str()].IsValid()) {
      return;
    }
  }

  is_valid_ = true;
}

bool ShaderBundle::IsValid() const {
  return is_valid_;
}

}  // namespace impeller
