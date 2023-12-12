// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/compiler/shader_bundle.h"

#include "flutter/testing/testing.h"
#include "impeller/compiler/types.h"

namespace impeller {
namespace compiler {
namespace testing {

const std::string kUnlitFragmentBundleConfig =
    "\"UnlitFragment\": {\"type\": \"fragment\", \"file\": "
    "\"shaders/flutter_gpu_unlit.frag\"}";
const std::string kUnlitVertexBundleConfig =
    "\"UnlitVertex\": {\"type\": \"vertex\", \"file\": "
    "\"shaders/flutter_gpu_unlit.vert\"}";

TEST(ShaderBundleTest, ParseShaderBundleConfigFailsForInvalidJSON) {
  std::string bundle = "";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_FALSE(result.has_value());
  ASSERT_STREQ(error.str().c_str(),
               "The shader bundle is not a valid JSON object.\n");
}

TEST(ShaderBundleTest, ParseShaderBundleConfigFailsWhenEntryNotObject) {
  std::string bundle = "{\"UnlitVertex\": []}";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_FALSE(result.has_value());
  ASSERT_STREQ(
      error.str().c_str(),
      "Invalid shader entry \"UnlitVertex\": Entry is not a JSON object.\n");
}

TEST(ShaderBundleTest, ParseShaderBundleConfigFailsWhenMissingFile) {
  std::string bundle = "{\"UnlitVertex\": {\"type\": \"vertex\"}}";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_FALSE(result.has_value());
  ASSERT_STREQ(error.str().c_str(),
               "Invalid shader entry \"UnlitVertex\": Missing required "
               "\"file\" field.\n");
}

TEST(ShaderBundleTest, ParseShaderBundleConfigFailsWhenMissingType) {
  std::string bundle =
      "{\"UnlitVertex\": {\"file\": \"shaders/flutter_gpu_unlit.vert\"}}";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_FALSE(result.has_value());
  ASSERT_STREQ(error.str().c_str(),
               "Invalid shader entry \"UnlitVertex\": Missing required "
               "\"type\" field.\n");
}

TEST(ShaderBundleTest, ParseShaderBundleConfigFailsForInvalidType) {
  std::string bundle =
      "{\"UnlitVertex\": {\"type\": \"invalid\", \"file\": "
      "\"shaders/flutter_gpu_unlit.vert\"}}";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_FALSE(result.has_value());
  ASSERT_STREQ(error.str().c_str(),
               "Invalid shader entry \"UnlitVertex\": Shader type "
               "\"invalid\" is unknown.\n");
}

TEST(ShaderBundleTest, ParseShaderBundleConfigFailsForInvalidLanguage) {
  std::string bundle =
      "{\"UnlitVertex\": {\"type\": \"vertex\", \"language\": \"invalid\", "
      "\"file\": \"shaders/flutter_gpu_unlit.vert\"}}";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_FALSE(result.has_value());
  ASSERT_STREQ(error.str().c_str(),
               "Invalid shader entry \"UnlitVertex\": Unknown language type "
               "\"invalid\".\n");
}

TEST(ShaderBundleTest, ParseShaderBundleConfigReturnsExpectedConfig) {
  std::string bundle =
      "{" + kUnlitVertexBundleConfig + ", " + kUnlitFragmentBundleConfig + "}";
  std::stringstream error;
  auto result = ParseShaderBundleConfig(bundle, error);
  ASSERT_TRUE(result.has_value());
  ASSERT_STREQ(error.str().c_str(), "");

  // NOLINTBEGIN(bugprone-unchecked-optional-access)
  auto maybe_vertex = result->find("UnlitVertex");
  auto maybe_fragment = result->find("UnlitFragment");
  ASSERT_TRUE(maybe_vertex != result->end());
  ASSERT_TRUE(maybe_fragment != result->end());
  auto vertex = maybe_vertex->second;
  auto fragment = maybe_fragment->second;
  // NOLINTEND(bugprone-unchecked-optional-access)

  EXPECT_EQ(vertex.type, SourceType::kVertexShader);
  EXPECT_EQ(vertex.language, SourceLanguage::kGLSL);
  EXPECT_STREQ(vertex.entry_point.c_str(), "main");
  EXPECT_STREQ(vertex.source_file_name.c_str(),
               "shaders/flutter_gpu_unlit.vert");

  EXPECT_EQ(fragment.type, SourceType::kFragmentShader);
  EXPECT_EQ(fragment.language, SourceLanguage::kGLSL);
  EXPECT_STREQ(fragment.entry_point.c_str(), "main");
  EXPECT_STREQ(fragment.source_file_name.c_str(),
               "shaders/flutter_gpu_unlit.frag");
}

}  // namespace testing
}  // namespace compiler
}  // namespace impeller
