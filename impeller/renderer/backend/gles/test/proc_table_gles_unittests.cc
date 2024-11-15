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

#define EXPECT_AVAILABLE(proc_ivar) \
  EXPECT_TRUE(mock_gles->GetProcTable().proc_ivar.IsAvailable());
#define EXPECT_UNAVAILABLE(proc_ivar) \
  EXPECT_FALSE(mock_gles->GetProcTable().proc_ivar.IsAvailable());

TEST(ProcTableGLES, ResolvesCorrectClearDepthProcOnES) {
  auto mock_gles = MockGLES::Init(std::nullopt, "OpenGL ES 3.0");
  EXPECT_TRUE(mock_gles->GetProcTable().GetDescription()->IsES());

  FOR_EACH_IMPELLER_ES_ONLY_PROC(EXPECT_AVAILABLE);
  FOR_EACH_IMPELLER_DESKTOP_ONLY_PROC(EXPECT_UNAVAILABLE);
}

TEST(ProcTableGLES, ResolvesCorrectClearDepthProcOnDesktopGL) {
  auto mock_gles = MockGLES::Init(std::nullopt, "OpenGL 4.0");
  EXPECT_FALSE(mock_gles->GetProcTable().GetDescription()->IsES());

  FOR_EACH_IMPELLER_DESKTOP_ONLY_PROC(EXPECT_AVAILABLE);
  FOR_EACH_IMPELLER_ES_ONLY_PROC(EXPECT_UNAVAILABLE);
}

TEST(ProcTableGLES, DebuggingOptionsAreRespected) {
#ifndef IMPELLER_DEBUG
  if ((true)) {
    GTEST_SKIP() << "Debugging is only available in IMPELLER_DEBUG";
  }
#endif  // IMPELLER_DEBUG
  auto mock_gles = MockGLES::Init(std::nullopt, "OpenGL ES 3.0");
  auto& gl = mock_gles->GetProcTable();
  // Check call logging.
  {
    gl.SetDebugGLCallLogging(false);
    ASSERT_FALSE(gl.Clear.log_calls);
    gl.SetDebugGLCallLogging(true);
    ASSERT_TRUE(gl.Clear.log_calls);
    gl.SetDebugGLCallLogging(false);
    ASSERT_FALSE(gl.Clear.log_calls);
    gl.SetDebugGLCallLogging(true, "glClear");
    ASSERT_TRUE(gl.Clear.log_calls);
    gl.SetDebugGLCallLogging(false, "glClear");
    ASSERT_FALSE(gl.Clear.log_calls);
  }
  // Check error checking.
  {
    gl.SetDebugGLErrorChecking(true);
    ASSERT_NE(gl.Clear.error_fn, nullptr);
    gl.SetDebugGLErrorChecking(false);
    ASSERT_EQ(gl.Clear.error_fn, nullptr);
    gl.SetDebugGLErrorChecking(true, "glClear");
    ASSERT_NE(gl.Clear.error_fn, nullptr);
    gl.SetDebugGLErrorChecking(false, "glClear");
    ASSERT_EQ(gl.Clear.error_fn, nullptr);
  }
}

}  // namespace testing
}  // namespace impeller
