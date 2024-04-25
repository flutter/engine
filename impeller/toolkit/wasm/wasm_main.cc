// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <emscripten.h>
#include "flutter/fml/logging.h"
#include "flutter/fml/message_loop.h"

int main(int argc, char const* argv[]) {
  fml::MessageLoop::EnsureInitializedForCurrentThread();
  fml::MessageLoop::GetCurrent().GetTaskRunner()->PostTask(
      []() { FML_LOG(IMPORTANT) << "This is a message."; });
  return 0;
}
