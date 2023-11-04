// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <future>
#include <memory>
#include <string_view>

#include "flutter/fml/macros.h"
#include "fml/mapping.h"
#include "impeller/core/shader_types.h"

namespace impeller {

class Context;
class ShaderFunction;

class ShaderLibrary : public std::enable_shared_from_this<ShaderLibrary> {
 public:
  virtual ~ShaderLibrary();

  virtual bool IsValid() const = 0;

  /// @brief Look up the [ShaderFunction] for the given [name], [stage], and
  ///        [specialization_constants].
  ///
  /// Specialization constants are only supported for fragment stage shaders.
  /// All other stages will ignore these values.
  virtual std::shared_ptr<const ShaderFunction> GetFunction(
      std::string_view name,
      ShaderStage stage,
      const std::vector<int32_t>& specialization_constants) = 0;

  using RegistrationCallback = std::function<void(bool)>;
  virtual void RegisterFunction(std::string name,
                                ShaderStage stage,
                                std::shared_ptr<fml::Mapping> code,
                                RegistrationCallback callback);

  /// @brief Unregister the [ShaderFunction] for the given [name], [stage], and
  ///        [specialization_constants].
  ///
  /// Specialization constants are only supported for fragment stage shaders.
  /// All other stages will ignore these values.
  virtual void UnregisterFunction(
      std::string name,
      ShaderStage stage,
      const std::vector<int32_t>& specialization_constants) = 0;

 protected:
  ShaderLibrary();

 private:
  ShaderLibrary(const ShaderLibrary&) = delete;

  ShaderLibrary& operator=(const ShaderLibrary&) = delete;
};

}  // namespace impeller
