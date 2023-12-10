// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/gpu/shader_library.h"

#include <utility>

#include "flutter/lib/gpu/fixtures.h"
#include "flutter/lib/gpu/shader.h"
#include "fml/mapping.h"
#include "fml/memory/ref_ptr.h"
#include "impeller/core/runtime_types.h"
#include "impeller/core/shader_types.h"
#include "impeller/renderer/vertex_descriptor.h"
#include "impeller/runtime_stage/runtime_stage.h"
#include "impeller/shader_bundle/shader_bundle_flatbuffers.h"
#include "runtime_stage_types_flatbuffers.h"

namespace flutter {
namespace gpu {

// ===[ BEGIN MEMES ]===========================================================
static fml::RefPtr<Shader> OpenRuntimeStageAsShader(
    std::shared_ptr<fml::Mapping> payload,
    const std::shared_ptr<impeller::VertexDescriptor>& vertex_desc) {
  impeller::RuntimeStage stage(std::move(payload));
  return Shader::Make(stage.GetEntrypoint(),
                      ToShaderStage(stage.GetShaderStage()),
                      stage.GetCodeMapping(), stage.GetUniforms(), vertex_desc);
}

static fml::RefPtr<ShaderLibrary> InstantiateTestShaderLibrary() {
  ShaderLibrary::ShaderMap shaders;
  {
    auto vertex_desc = std::make_shared<impeller::VertexDescriptor>();
    vertex_desc->SetStageInputs(
        FlutterGPUUnlitVertexShader::kAllShaderStageInputs,
        FlutterGPUUnlitVertexShader::kInterleavedBufferLayout);
    shaders["UnlitVertex"] = OpenRuntimeStageAsShader(
        std::make_shared<fml::NonOwnedMapping>(kFlutterGPUUnlitVertIPLR,
                                               kFlutterGPUUnlitVertIPLRLength),
        vertex_desc);
    shaders["UnlitFragment"] = OpenRuntimeStageAsShader(
        std::make_shared<fml::NonOwnedMapping>(kFlutterGPUUnlitFragIPLR,
                                               kFlutterGPUUnlitFragIPLRLength),
        nullptr);
  }
  {
    auto vertex_desc = std::make_shared<impeller::VertexDescriptor>();
    vertex_desc->SetStageInputs(
        FlutterGPUTextureVertexShader::kAllShaderStageInputs,
        FlutterGPUTextureVertexShader::kInterleavedBufferLayout);
    shaders["TextureVertex"] = OpenRuntimeStageAsShader(
        std::make_shared<fml::NonOwnedMapping>(
            kFlutterGPUTextureVertIPLR, kFlutterGPUTextureVertIPLRLength),
        vertex_desc);
    shaders["TextureFragment"] = OpenRuntimeStageAsShader(
        std::make_shared<fml::NonOwnedMapping>(
            kFlutterGPUTextureFragIPLR, kFlutterGPUTextureFragIPLRLength),
        nullptr);
  }
  auto library = ShaderLibrary::MakeFromShaders(std::move(shaders));
  return library;
}
// ===[ END MEMES ]=============================================================

IMPLEMENT_WRAPPERTYPEINFO(flutter_gpu, ShaderLibrary);

fml::RefPtr<ShaderLibrary> ShaderLibrary::override_shader_library_;

fml::RefPtr<ShaderLibrary> ShaderLibrary::MakeFromAsset(
    const std::string& name,
    std::string& out_error) {
  // ===========================================================================
  // This is a temporary hack to get the shader library populated in the
  // framework before the shader bundle format is landed!
  if (!override_shader_library_) {
    return InstantiateTestShaderLibrary();
  }
  // ===========================================================================

  if (override_shader_library_) {
    return override_shader_library_;
  }
  // TODO(bdero): Load the ShaderLibrary asset.
  out_error = "Shader bundle asset unimplemented";
  return nullptr;
}

fml::RefPtr<ShaderLibrary> ShaderLibrary::MakeFromShaders(ShaderMap shaders) {
  return fml::MakeRefCounted<flutter::gpu::ShaderLibrary>(nullptr,
                                                          std::move(shaders));
}

static impeller::ShaderType FromInputType(
    impeller::fb::InputDataType input_type) {
  switch (input_type) {
    case impeller::fb::InputDataType::kBoolean:
      return impeller::ShaderType::kBoolean;
    case impeller::fb::InputDataType::kSignedByte:
      return impeller::ShaderType::kSignedByte;
    case impeller::fb::InputDataType::kUnsignedByte:
      return impeller::ShaderType::kUnsignedByte;
    case impeller::fb::InputDataType::kSignedShort:
      return impeller::ShaderType::kSignedShort;
    case impeller::fb::InputDataType::kUnsignedShort:
      return impeller::ShaderType::kUnsignedShort;
    case impeller::fb::InputDataType::kSignedInt:
      return impeller::ShaderType::kSignedInt;
    case impeller::fb::InputDataType::kUnsignedInt:
      return impeller::ShaderType::kUnsignedInt;
    case impeller::fb::InputDataType::kSignedInt64:
      return impeller::ShaderType::kSignedInt64;
    case impeller::fb::InputDataType::kUnsignedInt64:
      return impeller::ShaderType::kUnsignedInt64;
    case impeller::fb::InputDataType::kFloat:
      return impeller::ShaderType::kFloat;
    case impeller::fb::InputDataType::kDouble:
      return impeller::ShaderType::kDouble;
  }
}

static size_t SizeOfInputType(impeller::fb::InputDataType input_type) {
  switch (input_type) {
    case impeller::fb::InputDataType::kBoolean:
      return 1;
    case impeller::fb::InputDataType::kSignedByte:
      return 1;
    case impeller::fb::InputDataType::kUnsignedByte:
      return 1;
    case impeller::fb::InputDataType::kSignedShort:
      return 2;
    case impeller::fb::InputDataType::kUnsignedShort:
      return 2;
    case impeller::fb::InputDataType::kSignedInt:
      return 4;
    case impeller::fb::InputDataType::kUnsignedInt:
      return 4;
    case impeller::fb::InputDataType::kSignedInt64:
      return 8;
    case impeller::fb::InputDataType::kUnsignedInt64:
      return 8;
    case impeller::fb::InputDataType::kFloat:
      return 4;
    case impeller::fb::InputDataType::kDouble:
      return 8;
  }
}

fml::RefPtr<ShaderLibrary> ShaderLibrary::MakeFromFlatbuffer(
    std::shared_ptr<fml::Mapping> payload) {
  if (payload == nullptr || !payload->GetMapping()) {
    return nullptr;
  }
  if (!impeller::fb::ShaderBundleBufferHasIdentifier(payload->GetMapping())) {
    return nullptr;
  }
  auto* bundle = impeller::fb::GetShaderBundle(payload->GetMapping());
  if (!bundle) {
    return nullptr;
  }

  ShaderLibrary::ShaderMap shader_map;

  for (const auto* bundled_shader : *bundle->shaders()) {
    const impeller::fb::RuntimeStage* runtime_stage = bundled_shader->shader();
    impeller::RuntimeStage stage(runtime_stage);

    std::shared_ptr<impeller::VertexDescriptor> vertex_descriptor = nullptr;
    if (stage.GetShaderStage() == impeller::RuntimeShaderStage::kVertex) {
      vertex_descriptor = std::make_shared<impeller::VertexDescriptor>();
      auto inputs_fb = runtime_stage->inputs();

      std::vector<impeller::ShaderStageIOSlot> inputs;
      inputs.reserve(inputs_fb->size());
      size_t default_stride = 0;
      for (const auto& input : *inputs_fb) {
        impeller::ShaderStageIOSlot slot;
        slot.name = input->name()->c_str();
        slot.location = input->location();
        slot.set = input->set();
        slot.binding = input->binding();
        slot.type = FromInputType(input->type());
        slot.bit_width = input->bit_width();
        slot.vec_size = input->vec_size();
        slot.columns = input->columns();
        slot.offset = input->offset();
        inputs.emplace_back(slot);

        default_stride +=
            SizeOfInputType(input->type()) * slot.vec_size * slot.columns;
      }
      std::vector<impeller::ShaderStageBufferLayout> layouts = {
          impeller::ShaderStageBufferLayout{
              .stride = default_stride,
              .binding = 0u,
          }};

      vertex_descriptor->SetStageInputs(inputs, layouts);
    }

    auto shader = flutter::gpu::Shader::Make(
        stage.GetEntrypoint(), ToShaderStage(stage.GetShaderStage()),
        stage.GetCodeMapping(), stage.GetUniforms(),
        std::move(vertex_descriptor));
    shader_map[bundled_shader->name()->str()] = std::move(shader);
  }

  return fml::MakeRefCounted<flutter::gpu::ShaderLibrary>(
      std::move(payload), std::move(shader_map));
}

void ShaderLibrary::SetOverride(
    fml::RefPtr<ShaderLibrary> override_shader_library) {
  override_shader_library_ = std::move(override_shader_library);
}

fml::RefPtr<Shader> ShaderLibrary::GetShader(const std::string& shader_name,
                                             Dart_Handle shader_wrapper) const {
  auto it = shaders_.find(shader_name);
  if (it == shaders_.end()) {
    return nullptr;  // No matching shaders.
  }
  auto shader = it->second;

  if (shader->dart_wrapper() == nullptr) {
    shader->AssociateWithDartWrapper(shader_wrapper);
  }
  return shader;
}

ShaderLibrary::ShaderLibrary(std::shared_ptr<fml::Mapping> payload,
                             ShaderMap shaders)
    : payload_(std::move(payload)), shaders_(std::move(shaders)) {}

ShaderLibrary::~ShaderLibrary() = default;

}  // namespace gpu
}  // namespace flutter

//----------------------------------------------------------------------------
/// Exports
///

Dart_Handle InternalFlutterGpu_ShaderLibrary_InitializeWithAsset(
    Dart_Handle wrapper,
    Dart_Handle asset_name) {
  if (!Dart_IsString(asset_name)) {
    return tonic::ToDart("Asset name must be a string");
  }

  std::string error;
  auto res = flutter::gpu::ShaderLibrary::MakeFromAsset(
      tonic::StdStringFromDart(asset_name), error);
  if (!res) {
    return tonic::ToDart(error);
  }
  res->AssociateWithDartWrapper(wrapper);
  return Dart_Null();
}

Dart_Handle InternalFlutterGpu_ShaderLibrary_GetShader(
    flutter::gpu::ShaderLibrary* wrapper,
    Dart_Handle shader_name,
    Dart_Handle shader_wrapper) {
  FML_DCHECK(Dart_IsString(shader_name));
  auto shader =
      wrapper->GetShader(tonic::StdStringFromDart(shader_name), shader_wrapper);
  if (!shader) {
    return Dart_Null();
  }
  return tonic::ToDart(shader.get());
}
