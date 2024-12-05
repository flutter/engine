// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"  // IWYU pragma: keep
#include "gtest/gtest.h"
#include "impeller/renderer/backend/gles/gpu_tracer_gles.h"
#include "impeller/renderer/backend/gles/test/mock_gles.h"

namespace impeller {
namespace testing {

using ::testing::_;

#ifdef IMPELLER_DEBUG
TEST(GPUTracerGLES, CanFormatFramebufferErrorMessage) {
  auto const extensions = std::vector<const char*>{
      "GL_KHR_debug",                 //
      "GL_EXT_disjoint_timer_query",  //
  };
  auto mock_gles_impl = std::make_unique<MockGLESImpl>();

  EXPECT_CALL(*mock_gles_impl, GenQueriesEXT(_, _))
      .WillRepeatedly([](GLsizei n, GLuint* ids) {
        for (int i = 0; i < n; ++i) {
          ids[i] = i + 1;
        }
      });
  EXPECT_CALL(*mock_gles_impl, BeginQueryEXT(_, _)).Times(2);
  EXPECT_CALL(*mock_gles_impl, EndQueryEXT(_));
  EXPECT_CALL(*mock_gles_impl, GetQueryObjectuivEXT(_, _, _))
      .WillRepeatedly(
          [](GLuint id, GLenum target, GLuint* result) { *result = GL_TRUE; });
  EXPECT_CALL(*mock_gles_impl, GetQueryObjectui64vEXT(_, _, _))
      .WillRepeatedly(
          [](GLuint id, GLenum target, GLuint64* result) { *result = 1000u; });
  EXPECT_CALL(*mock_gles_impl, DeleteQueriesEXT(_, _));

  std::shared_ptr<MockGLES> mock_gles =
      MockGLES::Init(std::move(mock_gles_impl), extensions);
  auto tracer =
      std::make_shared<GPUTracerGLES>(mock_gles->GetProcTable(), true);
  tracer->RecordRasterThread();
  tracer->MarkFrameStart(mock_gles->GetProcTable());
  tracer->MarkFrameEnd(mock_gles->GetProcTable());

  // auto calls = mock_gles->GetCapturedCalls();

  // std::vector<std::string> expected = {"glGenQueriesEXT", "glBeginQueryEXT",
  //                                      "glEndQueryEXT"};
  // for (auto i = 0; i < 3; i++) {
  //   EXPECT_EQ(calls[i], expected[i]);
  // }

  // Begin second frame, which prompts the tracer to query the result
  // from the previous frame.
  tracer->MarkFrameStart(mock_gles->GetProcTable());

  // calls = mock_gles->GetCapturedCalls();
  // std::vector<std::string> expected_b = {"glGetQueryObjectuivEXT",
  //                                        "glGetQueryObjectui64vEXT",
  //                                        "glDeleteQueriesEXT"};
  // for (auto i = 0; i < 3; i++) {
  //   EXPECT_EQ(calls[i], expected_b[i]);
  // }
}

#endif  // IMPELLER_DEBUG

}  // namespace testing
}  // namespace impeller
