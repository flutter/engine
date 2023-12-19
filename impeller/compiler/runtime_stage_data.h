// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_COMPILER_RUNTIME_STAGE_DATA_H_
#define FLUTTER_IMPELLER_COMPILER_RUNTIME_STAGE_DATA_H_

#include <memory>
#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/fml/mapping.h"
#include "impeller/compiler/types.h"
#include "runtime_stage_types_flatbuffers.h"
#include "spirv_parser.hpp"

namespace impeller {
namespace compiler {

class RuntimeStageData {
 public:
  RuntimeStageData(std::string entrypoint,
                   spv::ExecutionModel stage,
                   TargetPlatform target_platform);

  ~RuntimeStageData();

  void AddUniformDescription(UniformDescription uniform);

  void AddInputDescription(InputDescription input);

  void SetShaderData(std::shared_ptr<fml::Mapping> shader);

  void SetSkSLData(std::shared_ptr<fml::Mapping> sksl);

  std::unique_ptr<fb::RuntimeStageT> CreateFlatbuffer() const;

  std::shared_ptr<fml::Mapping> CreateMapping() const;

  std::shared_ptr<fml::Mapping> CreateJsonMapping() const;

 private:
  const std::string entrypoint_;
  const spv::ExecutionModel stage_;
  const TargetPlatform target_platform_;
  std::vector<UniformDescription> uniforms_;
  std::vector<InputDescription> inputs_;
  std::shared_ptr<fml::Mapping> shader_;
  std::shared_ptr<fml::Mapping> sksl_;

  RuntimeStageData(const RuntimeStageData&) = delete;

  RuntimeStageData& operator=(const RuntimeStageData&) = delete;
};

}  // namespace compiler
}  // namespace impeller

#endif  // FLUTTER_IMPELLER_COMPILER_RUNTIME_STAGE_DATA_H_
