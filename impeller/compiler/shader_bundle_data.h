// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_COMPILER_SHADER_BUNDLE_DATA_H_
#define FLUTTER_IMPELLER_COMPILER_SHADER_BUNDLE_DATA_H_

#include <memory>
#include <vector>

#include "flutter/fml/mapping.h"
#include "impeller/compiler/types.h"
#include "runtime_stage_types_flatbuffers.h"

namespace impeller {
namespace compiler {

class ShaderBundleData {
 public:
  ShaderBundleData(std::string entrypoint,
                   spv::ExecutionModel stage,
                   TargetPlatform target_platform);

  ~ShaderBundleData();

  void AddUniformDescription(UniformDescription uniform);

  void AddInputDescription(InputDescription input);

  void SetShaderData(std::shared_ptr<fml::Mapping> shader);

  void SetSkSLData(std::shared_ptr<fml::Mapping> sksl);

  std::unique_ptr<fb::RuntimeStageT> CreateFlatbuffer() const;

 private:
  const std::string entrypoint_;
  const spv::ExecutionModel stage_;
  const TargetPlatform target_platform_;
  std::vector<UniformDescription> uniforms_;
  std::vector<InputDescription> inputs_;
  std::shared_ptr<fml::Mapping> shader_;
  std::shared_ptr<fml::Mapping> sksl_;

  ShaderBundleData(const ShaderBundleData&) = delete;

  ShaderBundleData& operator=(const ShaderBundleData&) = delete;
};

}  // namespace compiler
}  // namespace impeller

#endif  // FLUTTER_IMPELLER_COMPILER_SHADER_BUNDLE_DATA_H_
