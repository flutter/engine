// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/compiler/shader_bundle.h"
#include "impeller/compiler/types.h"

#include "third_party/json/include/nlohmann/json.hpp"

namespace impeller {
namespace compiler {

static std::optional<ShaderBundleConfig> ParseShaderBundleConfig(
    std::string& json_config) {
  auto json = nlohmann::json::parse(json_config);
  if (!json.is_object()) {
    std::cerr << "The shader bundle must be a JSON object." << std::endl;
    return std::nullopt;
  }

  ShaderBundleConfig bundle;
  for (auto& [shader_name, shader_value] : json.items()) {
    if (bundle.find(shader_name) == bundle.end()) {
      std::cerr << "Duplicate shader \"" << shader_name << "\"." << std::endl;
      return std::nullopt;
    }
    if (!shader_value.is_object()) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Entry is not a JSON object." << std::endl;
      return std::nullopt;
    }

    ShaderConfig shader;

    if (!shader_value.contains("file")) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Missing required \"file\" field. \"" << std::endl;
      return std::nullopt;
    }
    shader.source_file_name = shader_value["file"];

    if (!shader_value.contains("type")) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Missing required \"type\" field. \"" << std::endl;
      return std::nullopt;
    }
    shader.type = shader_value["type"];
    if (shader.type == SourceType::kUnknown) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Shader type \"" << shader_value["type"]
                << "\" is unknown. \"" << std::endl;
      return std::nullopt;
    }

    shader.language = shader_value.contains("language")
                          ? ToSourceLanguage(shader_value["language"])
                          : SourceLanguage::kGLSL;
    if (shader.language == SourceLanguage::kUnknown) {
      std::cerr << "Invalid shader entry \"" << shader_name
                << "\": Unknown language type \"" << shader_value["language"]
                << "\"." << std::endl;
      return std::nullopt;
    }

    shader.entry_point = shader_value.contains("entry_point")
                             ? shader_value["entry_point"]
                             : "main";

    bundle[shader_name] = shader;
  }

  return bundle;
}

/// Parses the given JSON shader bundle configuration and invokes the compiler
/// multiple times to produce a shader bundle file.
bool GenerateShaderBundle(Switches& switches) {
  /// Parse the bundle files.
  std::optional<ShaderBundleConfig> bundle_config =
      ParseShaderBundleConfig(switches.iplr_bundle);
  if (!bundle_config) {
    return false;
  }

  return true;
}

}  // namespace compiler
}  // namespace impeller
