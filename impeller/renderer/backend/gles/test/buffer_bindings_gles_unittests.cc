// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"  // IWYU pragma: keep
#include "gtest/gtest.h"
#include "impeller/core/shader_types.h"
#include "impeller/renderer/backend/gles/buffer_bindings_gles.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"
#include "impeller/renderer/backend/gles/test/mock_gles.h"

namespace impeller {
namespace testing {

ShaderType type;
std::string name;
size_t offset;
size_t size;
size_t byte_length;
std::optional<size_t> array_elements;

TEST(BufferBindingsGLES, CanCacheLocationDataInShaderMetadata) {
  // Uniform metadata for
  // struct Foo {
  //   float Bar;
  //   float Baz;
  // }
  const ShaderMetadata metadata =
      ShaderMetadata{.name = "Foo",
                     .members = {ShaderStructMemberMetadata{
                                     .type = ShaderType::kFloat,
                                     .name = "Bar",
                                     .offset = 0u,
                                     .size = sizeof(Scalar),
                                     .byte_length = sizeof(Scalar),
                                     .array_elements = std::nullopt,
                                 },
                                 ShaderStructMemberMetadata{
                                     .type = ShaderType::kFloat,
                                     .name = "Baz",
                                     .offset = sizeof(Scalar),
                                     .size = sizeof(Scalar),
                                     .byte_length = sizeof(Scalar),
                                     .array_elements = std::nullopt,
                                 }}};
  // Uniform metadata for
  // sampler2D Fizz;
  const ShaderMetadata sampler_metadata = ShaderMetadata{.name = "Fizz"};

  auto buffer_bindings = std::make_shared<BufferBindingsGLES>();

  // Mock uniform locations.
  buffer_bindings->GetUniformLocationsForTesting()["FOO.BAR"] = 0;
  buffer_bindings->GetUniformLocationsForTesting()["FOO.BAZ"] = 1;
  buffer_bindings->GetUniformLocationsForTesting()["FIZZ"] = 2;

  // Lookup locations.

  auto bar_location = buffer_bindings->LookupUniformLocation(
      &metadata, metadata.members[0], false);
  auto baz_location = buffer_bindings->LookupUniformLocation(
      &metadata, metadata.members[1], false);
  auto fizz_location =
      buffer_bindings->LookupTextureLocation(&sampler_metadata);

  EXPECT_EQ(bar_location.value_or(-1), 0);
  EXPECT_EQ(baz_location.value_or(-1), 1);
  EXPECT_EQ(fizz_location, 2);

  // values have been cached.

  EXPECT_EQ(metadata.members[0].location.value_or(-1), 0);
  EXPECT_EQ(metadata.members[1].location.value_or(-1), 1);
  EXPECT_EQ(sampler_metadata.location.value_or(-1), 2);
}

}  // namespace testing
}  // namespace impeller