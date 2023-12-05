// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/compiler/shader_bundle.h"
#include "impeller/compiler/types.h"

#include "third_party/json/include/nlohmann/json.hpp"

namespace impeller {
namespace compiler {

/// Compiles and outputs a
bool GenerateShaderBundle(Switches& switches) {
  auto json = nlohmann::json::parse(switches.iplr_bundle);
  if (!json.is_object()) {
    std::cerr << "The shader bundle must be a JSON object." << std::endl;
    return false;
  }

  /// Bundle parsing.

  IPLRBundleEntries bundle_entries_result;
  for (auto& [bundle_name, bundle_value] : json.items()) {
    if (!bundle_value.is_object()) {
      std::cerr << "Invalid bundle entry \"" << bundle_name
                << "\": Entry is not a JSON object." << std::endl;
      return false;
    }

    /// Shader parsing.

    IPLRBundleEntry bundle_entry_result;
    for (auto& [shader_name, shader_value] : bundle_value.items()) {
      if (bundle_entry_result.find(shader_name) == bundle_entry_result.end()) {
        std::cerr << "Duplicate shader \"" << shader_name << "\" in bundle \""
                  << bundle_name << "\"." << std::endl;
        return false;
      }
      if (!bundle_value.is_object()) {
        std::cerr << "Invalid shader entry \"" << shader_name
                  << "\" in bundle \"" << bundle_name
                  << "\": Entry is not a JSON object." << std::endl;
        return false;
      }

      IPLRShaderEntry stage_result;

      stage_result.language = shader_value.contains("language")
                                  ? ToSourceLanguage(shader_value["language"])
                                  : SourceLanguage::kGLSL;
      if (stage_result.language == SourceLanguage::kUnknown) {
        std::cerr << "Invalid shader entry \"" << shader_name
                  << "\" in bundle \"" << bundle_name
                  << "\": Unknown language type \"" << shader_value["language"]
                  << "\"." << std::endl;
        return false;
      }

      if (!shader_value.contains("name")) {
        std::cerr << "Duplicate IPLR bundle stage entry \"" << shader_name
                  << "\" in bundle \"" << bundle_name << "\": Entry ."
                  << std::endl;
        return false;
      }
      stage_result.source_file_name = shader_value["name"];

      stage_result.entry_point = shader_value.contains("entry_point")
                                     ? shader_value["entry_point"]
                                     : "main";

      bundle_entry_result[shader_name] = stage_result;
    }
    bundle_entries_result[bundle_name] = bundle_entry_result;
  }

  return true;
}

}  // namespace compiler
}  // namespace impeller
