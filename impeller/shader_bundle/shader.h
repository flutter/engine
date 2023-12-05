// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <map>
#include <memory>

#include "flutter/fml/mapping.h"
#include "impeller/runtime_stage/runtime_stage.h"
#include "impeller/shader_bundle/shader_bundle_flatbuffers.h"

namespace impeller {

class ShaderBundle;

class BundledShader {
 public:
  // Note: Default constructor and copy operations required for map usage in
  // ShaderBundle.
  BundledShader();

  bool IsValid() const;

 private:
  bool is_valid_ = false;

  std::shared_ptr<RuntimeStage> runtime_stage_;

  explicit BundledShader(const fb::Shader* shader);

  friend ShaderBundle;
};

}  // namespace impeller
