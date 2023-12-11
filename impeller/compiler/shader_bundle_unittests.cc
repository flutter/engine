// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <initializer_list>
#include <vector>

#include "impeller/compiler/shader_bundle.h"

#include "flutter/testing/testing.h"

namespace impeller {
namespace compiler {
namespace testing {

const std::string kUnlitFragmentBundleConfig =
    "\"UnlitFragment\": {\"type\": \"fragment\", \"file\": "
    "\"shaders/flutter_gpu_unlit.frag\"}";
const std::string kUnlitVertexBundleConfig =
    "\"UnlitVertex\": {\"type\": \"vertex\", \"file\": "
    "\"shaders/flutter_gpu_unlit.vert\"}";

TEST(ShaderBundleTest, ShaderBundleConfigFailsForInvalidJSON) {
  std::string bundle = "";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_FALSE(result.has_value());
  ASSERT_STREQ(error.str().c_str(), "The shader bundle must be a JSON object.");
}

TEST(ShaderBundleTest, ShaderBundleConfigFailsForDuplicateShaders) {
  std::string bundle = "{" + kUnlitFragmentBundleConfig + ", " +
                       kUnlitFragmentBundleConfig + "}";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_FALSE(result.has_value());
  ASSERT_STREQ(error.str().c_str(), "Duplicate shader \"UnlitFragment\".");
}

TEST(ShaderBundleTest, ShaderBundleConfigFailsWhenEntryNotObject) {
  std::string bundle = "\"UnlitVertex\": []";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_FALSE(result.has_value());
  ASSERT_STREQ(
      error.str().c_str(),
      "Invalid shader entry \"UnlitFragment\": Entry is not a JSON object.");
}

}  // namespace testing
}  // namespace compiler
}  // namespace impeller
