// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/gpu/shader.h"

#include <utility>

#include "fml/make_copyable.h"
#include "impeller/renderer/shader_library.h"

namespace flutter {
namespace gpu {

IMPLEMENT_WRAPPERTYPEINFO(flutter_gpu, Shader);

Shader::Shader() = default;

Shader::~Shader() = default;

fml::RefPtr<Shader> Shader::Make(
    std::string entrypoint,
    impeller::ShaderStage stage,
    std::shared_ptr<fml::Mapping> code_mapping,
    std::vector<impeller::RuntimeUniformDescription> uniforms) {
  auto shader = fml::MakeRefCounted<Shader>();
  shader->entrypoint_ = std::move(entrypoint);
  shader->stage_ = stage;
  shader->code_mapping_ = std::move(code_mapping);
  shader->uniforms_ = std::move(uniforms);
  return shader;
}

bool Shader::IsRegistered(Context& context) {
  auto& lib = *context.GetContext()->GetShaderLibrary();
  return lib.GetFunction(entrypoint_, stage_) != nullptr;
}

bool Shader::RegisterSync(Context& context) {
  if (IsRegistered(context)) {
    return true;  // Already registered.
  }

  auto& lib = *context.GetContext()->GetShaderLibrary();

  std::promise<bool> promise;
  auto future = promise.get_future();
  lib.RegisterFunction(
      entrypoint_, stage_, code_mapping_,
      fml::MakeCopyable([promise = std::move(promise)](bool result) mutable {
        promise.set_value(result);
      }));
  if (!future.get()) {
    return false;  // Registration failed.
  }
  return true;
}

}  // namespace gpu
}  // namespace flutter

//----------------------------------------------------------------------------
/// Exports
///
