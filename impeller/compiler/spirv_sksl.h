// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <memory>
#include <utility>
#include <variant>

#include "flutter/fml/logging.h"
#include "flutter/fml/macros.h"
#include "spirv_glsl.hpp"

namespace impeller {
namespace compiler {

class CompilerSkSL : public spirv_cross::CompilerGLSL {
 public:
  explicit CompilerSkSL(std::vector<uint32_t> spirv_)
      : CompilerGLSL(std::move(spirv_)) {}

  CompilerSkSL(const uint32_t* ir_, size_t word_count)
      : CompilerGLSL(ir_, word_count) {}

  explicit CompilerSkSL(const spirv_cross::ParsedIR& ir_)
      : spirv_cross::CompilerGLSL(ir_) {}

  explicit CompilerSkSL(spirv_cross::ParsedIR&& ir_)
      : spirv_cross::CompilerGLSL(std::move(ir_)) {}

  std::string compile() override;

 private:
  void emit_header() override;

  std::string type_to_glsl(const spirv_cross::SPIRType& type,
                           uint32_t id = 0) override;

  std::string builtin_to_glsl(spv::BuiltIn builtin,
                              spv::StorageClass storage) override;

  void emit_uniform(const spirv_cross::SPIRVariable& var) override;
};

}  // namespace compiler
}  // namespace impeller
