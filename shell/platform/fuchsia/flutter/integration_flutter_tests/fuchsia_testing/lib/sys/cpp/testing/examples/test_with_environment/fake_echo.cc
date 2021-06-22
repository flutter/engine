// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fake_echo.h"

#include <lib/async-loop/cpp/loop.h>
#include <lib/async-loop/default.h>
#include <lib/sys/cpp/component_context.h>

int main(int argc, char** argv) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <answer>\n", argv[0]);
    return 1;
  }
  async::Loop loop(&kAsyncLoopConfigAttachToCurrentThread);
  echo::testing::FakeEcho echo;
  echo.SetAnswer(argv[1]);
  auto context = sys::ComponentContext::CreateAndServeOutgoingDirectory();
  context->outgoing()->AddPublicService(echo.GetHandler());
  loop.Run();
  return 0;
}
