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

TEST(ShaderBundleTest, ShaderBundleConfigFailsForInvalidJSON) {
  std::string bundle = "";
  auto result = ParseShaderBundleConfig(bundle);
  ASSERT_FALSE(result.has_value());
}

}  // namespace testing
}  // namespace compiler
}  // namespace impeller
