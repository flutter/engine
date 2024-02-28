// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <optional>

#include "flutter/testing/testing.h"  // IWYU pragma: keep
#include "gtest/gtest.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"
#include "impeller/renderer/backend/gles/test/mock_gles.h"

namespace impeller {
namespace testing {

static void FakeClearDepthf(GLfloat depth) {}
static void FakeClearDepth(GLdouble depth) {}
static auto kClearDepthResolver =
    ProcTableGLES::Resolver([](const char* name) -> void* {
      if (strcmp(name, "glClearDepthf") == 0) {
        return reinterpret_cast<void*>(FakeClearDepthf);
      }
      if (strcmp(name, "glClearDepth") == 0) {
        return reinterpret_cast<void*>(FakeClearDepth);
      }
      return kMockResolverGLES(name);
    });

TEST(ProcTableGLES, ResolvesCorrectClearDepthProcOnES) {
  auto mock_gles =
      MockGLES::Init(std::nullopt, "OpenGL ES 3.0", kClearDepthResolver);
  EXPECT_TRUE(mock_gles->GetProcTable().GetDescription()->IsES());
  EXPECT_EQ(mock_gles->GetProcTable().ClearDepthf.function, FakeClearDepthf);
}

TEST(ProcTableGLES, ResolvesCorrectClearDepthProcOnDesktopGL) {
  auto mock_gles =
      MockGLES::Init(std::nullopt, "OpenGL 4.0", kClearDepthResolver);
  EXPECT_FALSE(mock_gles->GetProcTable().GetDescription()->IsES());
  EXPECT_EQ(mock_gles->GetProcTable().ClearDepthf.function, FakeClearDepth);
}

}  // namespace testing
}  // namespace impeller
