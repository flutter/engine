// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <map>
#include <memory>

#include "flutter/fml/mapping.h"
#include "impeller/runtime_stage/runtime_stage.h"
#include "impeller/shader_bundle/shader.h"

namespace impeller {

class ShaderBundle {
 public:
  explicit ShaderBundle(std::shared_ptr<fml::Mapping> payload);

  ~ShaderBundle();
  ShaderBundle(ShaderBundle&&);

  bool IsValid() const;

 private:
  bool is_valid_;

  std::shared_ptr<fml::Mapping> payload_;
  std::map<std::string, BundledShader> shaders_;

  ShaderBundle(const ShaderBundle&) = delete;
  ShaderBundle& operator=(const ShaderBundle&) = delete;
};

}  // namespace impeller
