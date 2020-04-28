// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_TEST_RENDERER_CONTEXT_H_
#define FLUTTER_SHELL_TEST_RENDERER_CONTEXT_H_

#include "gtest/gtest.h"
#include "renderer_context_manager.h"

namespace flutter {
namespace testing {

//------------------------------------------------------------------------------
/// The renderer context used for testing
class TestRendererContext : public RendererContext, public ::testing::Test {
 public:
  //------------------------------------------------------------------------------
  /// Represents the current alive context.
  ///
  /// -1 represents no context has been set.
  static int currentContext;

  TestRendererContext(int context);

  ~TestRendererContext() override;

  bool SetCurrent() override;

  void RemoveCurrent() override;

  int GetContext();

  //------------------------------------------------------------------------------
  /// Set the current context without going through the |RendererContextManager|.
  ///
  /// This is to mimic how other programs outside flutter sets the context.
  static void SetCurrentContext(int context);

 private:
  int context_;
};
}  // namespace testing
}  // namespace flutter
#endif  // FLUTTER_SHELL_TEST_RENDERER_CONTEXT_H_