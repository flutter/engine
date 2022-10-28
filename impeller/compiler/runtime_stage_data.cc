// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/compiler/runtime_stage_data.h"

#include <array>
#include <optional>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "impeller/base/validation.h"
#include "impeller/runtime_stage/runtime_stage_flatbuffers.h"

namespace impeller {
namespace compiler {

RuntimeStageData::RuntimeStageData(std::string entrypoint,
                                   spv::ExecutionModel stage,
                                   TargetPlatform target_platform)
    : entrypoint_(std::move(entrypoint)),
      stage_(stage),
      target_platform_(target_platform) {}

RuntimeStageData::~RuntimeStageData() = default;

void RuntimeStageData::AddUniformDescription(UniformDescription uniform) {
  uniforms_.emplace_back(std::move(uniform));
}

void RuntimeStageData::SetShaderData(std::shared_ptr<fml::Mapping> shader) {
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
    case spv::ExecutionModel::ExecutionModelTessellationControl:
      return fb::Stage::kTessellationControl;
    case spv::ExecutionModel::ExecutionModelTessellationEvaluation:
      return fb::Stage::kTessellationEvaluation;
    default:
      return std::nullopt;
  }
  FML_UNREACHABLE();
}

static std::optional<uint32_t> ToJsonStage(spv::ExecutionModel stage) {
  switch (stage) {
    case spv::ExecutionModel::ExecutionModelVertex:
      return 0;  // fb::Stage::kVertex;
    case spv::ExecutionModel::ExecutionModelFragment:
      return 1;  // fb::Stage::kFragment;
    case spv::ExecutionModel::ExecutionModelGLCompute:
      return 2;  // fb::Stage::kCompute;
    case spv::ExecutionModel::ExecutionModelTessellationControl:
      return 3;  // fb::Stage::kTessellationControl;
    case spv::ExecutionModel::ExecutionModelTessellationEvaluation:
      return 4;  // fb::Stage::kTessellationEvaluation;
    default:
      return std::nullopt;
  }
  FML_UNREACHABLE();
}

static std::optional<fb::TargetPlatform> ToTargetPlatform(
    TargetPlatform platform) {
  switch (platform) {
    case TargetPlatform::kUnknown:
    case TargetPlatform::kMetalDesktop:
    case TargetPlatform::kMetalIOS:
    case TargetPlatform::kOpenGLES:
    case TargetPlatform::kOpenGLDesktop:
    case TargetPlatform::kVulkan:
      return std::nullopt;
    case TargetPlatform::kSkSL:
      return fb::TargetPlatform::kSkSL;
    case TargetPlatform::kRuntimeStageMetal:
      return fb::TargetPlatform::kMetal;
    case TargetPlatform::kRuntimeStageGLES:
      return fb::TargetPlatform::kOpenGLES;
  }
  FML_UNREACHABLE();
}

static std::optional<uint32_t> ToJsonTargetPlatform(TargetPlatform platform) {
  switch (platform) {
    case TargetPlatform::kUnknown:
    case TargetPlatform::kMetalDesktop:
    case TargetPlatform::kMetalIOS:
    case TargetPlatform::kOpenGLES:
    case TargetPlatform::kOpenGLDesktop:
    case TargetPlatform::kVulkan:
      return std::nullopt;
    case TargetPlatform::kSkSL:
      return 0;  // fb::TargetPlatform::kSkSL;
    case TargetPlatform::kRuntimeStageMetal:
      return 1;  // fb::TargetPlatform::kMetal;
    case TargetPlatform::kRuntimeStageGLES:
      return 2;  // fb::TargetPlatform::kOpenGLES;
  }
  FML_UNREACHABLE();
}

static std::optional<fb::UniformDataType> ToType(
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

static std::optional<uint32_t> ToJsonType(
    spirv_cross::SPIRType::BaseType type) {
  switch (type) {
    case spirv_cross::SPIRType::Boolean:
      return 0;  // fb::UniformDataType::kBoolean;
    case spirv_cross::SPIRType::SByte:
      return 1;  // fb::UniformDataType::kSignedByte;
    case spirv_cross::SPIRType::UByte:
      return 2;  // fb::UniformDataType::kUnsignedByte;
    case spirv_cross::SPIRType::Short:
      return 3;  // fb::UniformDataType::kSignedShort;
    case spirv_cross::SPIRType::UShort:
      return 4;  // fb::UniformDataType::kUnsignedShort;
    case spirv_cross::SPIRType::Int:
      return 5;  // fb::UniformDataType::kSignedInt;
    case spirv_cross::SPIRType::UInt:
      return 6;  // fb::UniformDataType::kUnsignedInt;
    case spirv_cross::SPIRType::Int64:
      return 7;  // fb::UniformDataType::kSignedInt64;
    case spirv_cross::SPIRType::UInt64:
      return 8;  // fb::UniformDataType::kUnsignedInt64;
    case spirv_cross::SPIRType::Half:
      return 9;  // b::UniformDataType::kHalfFloat;
    case spirv_cross::SPIRType::Float:
      return 10;  // fb::UniformDataType::kFloat;
    case spirv_cross::SPIRType::Double:
      return 11;  // fb::UniformDataType::kDouble;
    case spirv_cross::SPIRType::SampledImage:
      return 12;  // fb::UniformDataType::kSampledImage;
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

static const char* kStageKey = "stage";
static const char* kTargetPlatformKey = "target_platform";
static const char* kEntrypointKey = "entrypoint";
static const char* kUniformsKey = "uniforms";
static const char* kShaderKey = "shader";
static const char* kUniformNameKey = "name";
static const char* kUniformLocationKey = "location";
static const char* kUniformTypeKey = "type";
static const char* kUniformRowsKey = "rows";
static const char* kUniformColumnsKey = "columns";
static const char* kUniformBitWidthKey = "bit_width";
static const char* kUniformArrayElementsKey = "array_elements";

std::shared_ptr<fml::Mapping> RuntimeStageData::CreateJsonMapping() const {
  using rapidjson::StringBuffer;
  using rapidjson::Writer;

  // Runtime Stage JSON format
  //   {
  //      "stage": 0,
  //      "target_platform": "",
  //      "entrypoint": "",
  //      "shader": "",
  //      "uniforms": [
  //        {
  //           "name": "..",
  //           "location": 0,
  //           "type": 0,
  //           "rows": 0,
  //           "columns": 0,
  //           "bit_width": 0,
  //           "array_elements": 0,
  //        }
  //      ]
  //   },
  StringBuffer s;
  Writer<StringBuffer> writer(s);
  writer.StartObject();

  const auto stage = ToJsonStage(stage_);
  if (!stage.has_value()) {
    VALIDATION_LOG << "Invalid runtime stage.";
    return nullptr;
  }
  writer.Key(kStageKey);
  writer.Int64(stage.value());

  const auto target_platform = ToJsonTargetPlatform(target_platform_);
  if (!target_platform.has_value()) {
    VALIDATION_LOG << "Invalid target platform for runtime stage.";
    return nullptr;
  }
  writer.Key(kTargetPlatformKey);
  writer.Int64(target_platform.value());

  if (shader_->GetSize() > 0u) {
    std::string shader(reinterpret_cast<const char*>(shader_->GetMapping()),
                       shader_->GetSize());
    writer.Key(kShaderKey);
    writer.String(shader.c_str());
  }

  writer.Key(kUniformsKey);
  writer.StartArray();
  for (const auto& uniform : uniforms_) {
    writer.StartObject();
    writer.Key(kUniformNameKey);
    writer.String(uniform.name.c_str());
    writer.Key(kUniformLocationKey);
    writer.Int64(uniform.location);
    writer.Key(kUniformRowsKey);
    writer.Int64(uniform.rows);
    writer.Key(kUniformColumnsKey);
    writer.Int64(uniform.columns);

    auto uniform_type = ToJsonType(uniform.type);
    if (!uniform_type.has_value()) {
      VALIDATION_LOG << "Invalid uniform type for runtime stage.";
      return nullptr;
    }

    writer.Key(kUniformTypeKey);
    writer.Int64(uniform_type.value());
    writer.Key(kUniformBitWidthKey);
    writer.Int64(uniform.bit_width);
    writer.Key(kUniformArrayElementsKey);
    if (uniform.array_elements.has_value()) {
      writer.Int64(uniform.array_elements.value());
    } else {
      writer.Int64(0);
    }
    writer.EndObject();
  }
  writer.EndArray();
  writer.EndObject();
  return std::make_shared<fml::DataMapping>(s.GetString());
}

std::shared_ptr<fml::Mapping> RuntimeStageData::CreateMapping() const {
  // The high level object API is used here for writing to the buffer. This is
  // just a convenience.
  fb::RuntimeStageT runtime_stage;
  runtime_stage.entrypoint = entrypoint_;
  const auto stage = ToStage(stage_);
  if (!stage.has_value()) {
    VALIDATION_LOG << "Invalid runtime stage.";
    return nullptr;
  }
  runtime_stage.stage = stage.value();
  const auto target_platform = ToTargetPlatform(target_platform_);
  if (!target_platform.has_value()) {
    VALIDATION_LOG << "Invalid target platform for runtime stage.";
    return nullptr;
  }
  runtime_stage.target_platform = target_platform.value();
  if (!shader_) {
    VALIDATION_LOG << "No shader specified for runtime stage.";
    return nullptr;
  }
  if (shader_->GetSize() > 0u) {
    runtime_stage.shader = {shader_->GetMapping(),
                            shader_->GetMapping() + shader_->GetSize()};
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
    auto uniform_type = ToType(uniform.type);
    if (!uniform_type.has_value()) {
      VALIDATION_LOG << "Invalid uniform type for runtime stage.";
      return nullptr;
    }
    desc->type = uniform_type.value();
    desc->bit_width = uniform.bit_width;
    if (uniform.array_elements.has_value()) {
      desc->array_elements = uniform.array_elements.value();
    }

    runtime_stage.uniforms.emplace_back(std::move(desc));
  }
  auto builder = std::make_shared<flatbuffers::FlatBufferBuilder>();
  builder->Finish(fb::RuntimeStage::Pack(*builder.get(), &runtime_stage),
                  fb::RuntimeStageIdentifier());
  return std::make_shared<fml::NonOwnedMapping>(builder->GetBufferPointer(),
                                                builder->GetSize(),
                                                [builder](auto, auto) {});
}

}  // namespace compiler
}  // namespace impeller
