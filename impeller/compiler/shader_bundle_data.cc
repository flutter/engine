// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/compiler/shader_bundle_data.h"

#include <array>
#include <cstdint>
#include <optional>

#include "inja/inja.hpp"

#include "impeller/base/validation.h"
#include "impeller/shader_bundle/shader_bundle_flatbuffers.h"
#include "runtime_stage_types_flatbuffers.h"

namespace impeller {
namespace compiler {

ShaderBundleData::ShaderBundleData(std::string entrypoint,
                                   spv::ExecutionModel stage,
                                   TargetPlatform target_platform)
    : entrypoint_(std::move(entrypoint)),
      stage_(stage),
      target_platform_(target_platform) {}

ShaderBundleData::~ShaderBundleData() = default;

void ShaderBundleData::AddUniformDescription(UniformDescription uniform) {
  uniforms_.emplace_back(std::move(uniform));
}

void ShaderBundleData::AddInputDescription(InputDescription input) {
  inputs_.emplace_back(std::move(input));
}

void ShaderBundleData::SetShaderData(std::shared_ptr<fml::Mapping> shader) {
  shader_ = std::move(shader);
}

static std::optional<fb::Stage> ToStage(spv::ExecutionModel stage) {
  switch (stage) {
    case spv::ExecutionModel::ExecutionModelVertex:
      return fb::Stage::kVertex;
    case spv::ExecutionModel::ExecutionModelFragment:
      return fb::Stage::kFragment;
    case spv::ExecutionModel::ExecutionModelGLCompute:
      return fb::Stage::kCompute;
    default:
      return std::nullopt;
  }
  FML_UNREACHABLE();
}

static std::optional<fb::UniformDataType> ToUniformType(
    spirv_cross::SPIRType::BaseType type) {
  switch (type) {
    case spirv_cross::SPIRType::Boolean:
      return fb::UniformDataType::kBoolean;
    case spirv_cross::SPIRType::SByte:
      return fb::UniformDataType::kSignedByte;
    case spirv_cross::SPIRType::UByte:
      return fb::UniformDataType::kUnsignedByte;
    case spirv_cross::SPIRType::Short:
      return fb::UniformDataType::kSignedShort;
    case spirv_cross::SPIRType::UShort:
      return fb::UniformDataType::kUnsignedShort;
    case spirv_cross::SPIRType::Int:
      return fb::UniformDataType::kSignedInt;
    case spirv_cross::SPIRType::UInt:
      return fb::UniformDataType::kUnsignedInt;
    case spirv_cross::SPIRType::Int64:
      return fb::UniformDataType::kSignedInt64;
    case spirv_cross::SPIRType::UInt64:
      return fb::UniformDataType::kUnsignedInt64;
    case spirv_cross::SPIRType::Half:
      return fb::UniformDataType::kHalfFloat;
    case spirv_cross::SPIRType::Float:
      return fb::UniformDataType::kFloat;
    case spirv_cross::SPIRType::Double:
      return fb::UniformDataType::kDouble;
    case spirv_cross::SPIRType::SampledImage:
      return fb::UniformDataType::kSampledImage;
    case spirv_cross::SPIRType::AccelerationStructure:
    case spirv_cross::SPIRType::AtomicCounter:
    case spirv_cross::SPIRType::Char:
    case spirv_cross::SPIRType::ControlPointArray:
    case spirv_cross::SPIRType::Image:
    case spirv_cross::SPIRType::Interpolant:
    case spirv_cross::SPIRType::RayQuery:
    case spirv_cross::SPIRType::Sampler:
    case spirv_cross::SPIRType::Struct:
    case spirv_cross::SPIRType::Unknown:
    case spirv_cross::SPIRType::Void:
      return std::nullopt;
  }
  FML_UNREACHABLE();
}
static std::optional<fb::InputDataType> ToInputType(
    spirv_cross::SPIRType::BaseType type) {
  switch (type) {
    case spirv_cross::SPIRType::Boolean:
      return fb::InputDataType::kBoolean;
    case spirv_cross::SPIRType::SByte:
      return fb::InputDataType::kSignedByte;
    case spirv_cross::SPIRType::UByte:
      return fb::InputDataType::kUnsignedByte;
    case spirv_cross::SPIRType::Short:
      return fb::InputDataType::kSignedShort;
    case spirv_cross::SPIRType::UShort:
      return fb::InputDataType::kUnsignedShort;
    case spirv_cross::SPIRType::Int:
      return fb::InputDataType::kSignedInt;
    case spirv_cross::SPIRType::UInt:
      return fb::InputDataType::kUnsignedInt;
    case spirv_cross::SPIRType::Int64:
      return fb::InputDataType::kSignedInt64;
    case spirv_cross::SPIRType::UInt64:
      return fb::InputDataType::kUnsignedInt64;
    case spirv_cross::SPIRType::Float:
      return fb::InputDataType::kFloat;
    case spirv_cross::SPIRType::Double:
      return fb::InputDataType::kDouble;
    case spirv_cross::SPIRType::Unknown:
    case spirv_cross::SPIRType::Void:
    case spirv_cross::SPIRType::Half:
    case spirv_cross::SPIRType::AtomicCounter:
    case spirv_cross::SPIRType::Struct:
    case spirv_cross::SPIRType::Image:
    case spirv_cross::SPIRType::SampledImage:
    case spirv_cross::SPIRType::Sampler:
    case spirv_cross::SPIRType::AccelerationStructure:
    case spirv_cross::SPIRType::RayQuery:
    case spirv_cross::SPIRType::ControlPointArray:
    case spirv_cross::SPIRType::Interpolant:
    case spirv_cross::SPIRType::Char:
      return std::nullopt;
  }
  FML_UNREACHABLE();
}

std::unique_ptr<fb::RuntimeStageT> ShaderBundleData::CreateFlatbuffer() const {
  auto shader_bundle = std::make_unique<fb::RuntimeStageT>();

  // The high level object API is used here for writing to the buffer. This is
  // just a convenience.
  shader_bundle->entrypoint = entrypoint_;
  const auto stage = ToStage(stage_);
  if (!stage.has_value()) {
    VALIDATION_LOG << "Invalid shader bundle.";
    return nullptr;
  }
  shader_bundle->stage = stage.value();
  // This field is ignored, so just set it to anything.
  shader_bundle->target_platform = fb::TargetPlatform::kMetal;
  if (!shader_) {
    VALIDATION_LOG << "No shader specified for shader bundle.";
    return nullptr;
  }
  if (shader_->GetSize() > 0u) {
    shader_bundle->shader = {shader_->GetMapping(),
                             shader_->GetMapping() + shader_->GetSize()};
  }
  // It is not an error for the SkSL to be ommitted.
  if (sksl_ && sksl_->GetSize() > 0u) {
    shader_bundle->sksl = {sksl_->GetMapping(),
                           sksl_->GetMapping() + sksl_->GetSize()};
  }
  for (const auto& uniform : uniforms_) {
    auto desc = std::make_unique<fb::UniformDescriptionT>();

    desc->name = uniform.name;
    if (desc->name.empty()) {
      VALIDATION_LOG << "Uniform name cannot be empty.";
      return nullptr;
    }
    desc->location = uniform.location;
    desc->rows = uniform.rows;
    desc->columns = uniform.columns;
    auto uniform_type = ToUniformType(uniform.type);
    if (!uniform_type.has_value()) {
      VALIDATION_LOG << "Invalid uniform type for runtime stage.";
      return nullptr;
    }
    desc->type = uniform_type.value();
    desc->bit_width = uniform.bit_width;
    if (uniform.array_elements.has_value()) {
      desc->array_elements = uniform.array_elements.value();
    }

    shader_bundle->uniforms.emplace_back(std::move(desc));
  }

  for (const auto& input : inputs_) {
    auto desc = std::make_unique<fb::StageInputT>();

    desc->name = input.name;

    if (desc->name.empty()) {
      VALIDATION_LOG << "Stage input name cannot be empty.";
      return nullptr;
    }
    desc->location = input.location;
    desc->set = input.set;
    desc->binding = input.binding;
    auto input_type = ToInputType(input.type);
    if (!input_type.has_value()) {
      VALIDATION_LOG << "Invalid uniform type for runtime stage.";
      return nullptr;
    }
    desc->type = input_type.value();
    desc->bit_width = input.bit_width;
    desc->vec_size = input.vec_size;
    desc->columns = input.columns;
    desc->offset = input.offset;

    shader_bundle->inputs.emplace_back(std::move(desc));
  }

  return shader_bundle;
}

}  // namespace compiler
}  // namespace impeller
