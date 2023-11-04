// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <future>
#include <memory>
#include <string>
#include <unordered_map>

#include "flutter/fml/hash_combine.h"
#include "flutter/fml/macros.h"
#include "impeller/core/shader_types.h"

namespace impeller {

struct ShaderKey {
  std::string name;
  ShaderStage stage = ShaderStage::kUnknown;
  std::vector<int32_t> specialization_constants = {};

  ShaderKey(std::string_view p_name,
            ShaderStage p_stage,
            const std::vector<int32_t>& p_specialization_constants)
      : name({p_name.data(), p_name.size()}),
        stage(p_stage),
        specialization_constants(p_specialization_constants) {}

  struct Hash {
    size_t operator()(const ShaderKey& key) const {
      auto seed = fml::HashCombine();
      for (auto constant : key.specialization_constants) {
        seed = fml::HashCombine(seed, constant);
      }
      return fml::HashCombine(key.name, key.stage, seed);
    }
  };

  struct Equal {
    constexpr bool operator()(const ShaderKey& k1, const ShaderKey& k2) const {
      return k1.stage == k2.stage && k1.name == k2.name &&
             k1.specialization_constants == k2.specialization_constants;
    }
  };
};

class ShaderFunction;

using ShaderFunctionMap =
    std::unordered_map<ShaderKey,
                       std::shared_ptr<const ShaderFunction>,
                       ShaderKey::Hash,
                       ShaderKey::Equal>;

}  // namespace impeller
