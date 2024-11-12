// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <optional>

#include "flutter/testing/testing.h"  // IWYU pragma: keep
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "impeller/playground/playground_test.h"
#include "impeller/renderer/backend/gles/context_gles.h"
#include "impeller/renderer/backend/gles/proc_table_gles.h"
#include "impeller/renderer/backend/gles/test/mock_gles.h"

namespace impeller {
namespace testing {

#define EXPECT_AVAILABLE(proc_ivar) \
  EXPECT_TRUE(mock_gles->GetProcTable().proc_ivar.IsAvailable());
#define EXPECT_UNAVAILABLE(proc_ivar) \
  EXPECT_FALSE(mock_gles->GetProcTable().proc_ivar.IsAvailable());

using ProcTablePlaygroundTest = PlaygroundTest;
INSTANTIATE_OPENGLES_PLAYGROUND_SUITE(ProcTablePlaygroundTest);

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

TEST_P(ProcTablePlaygroundTest, DescribeFramebuffer) {
  FML_LOG(ERROR) << "*** " << __PRETTY_FUNCTION__;
  auto context = GetContext();
  ASSERT_TRUE(context);

  const auto& gl_context = ContextGLES::Cast(*context);
  const auto& gl = gl_context.GetReactor()->GetProcTable();

  GLuint fbo;
  gl.GenFramebuffers(1, &fbo);
  gl.BindFramebuffer(GL_FRAMEBUFFER, fbo);

  GLuint texture;
  gl.GenTextures(1, &texture);
  gl.BindTexture(GL_TEXTURE_2D, texture);
  gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 100, 100, 0, GL_RGBA,
                GL_UNSIGNED_BYTE, nullptr);
  gl.BindTexture(GL_TEXTURE_2D, 0);

  gl.FramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                          texture, 0);

  std::string description = gl.DescribeCurrentFramebuffer();
  FML_LOG(ERROR) << "[" << description << "]";

  EXPECT_THAT(description,
              ::testing::MatchesRegex("FBO.*GL_FRAMEBUFFER_COMPLETE.*"
                                      "Color Attachment: GL_TEXTURE.*"
                                      "Depth Attachment: No Attachment.*"
                                      "Stencil Attachment: No Attachment.*"));
}

}  // namespace testing
}  // namespace impeller
