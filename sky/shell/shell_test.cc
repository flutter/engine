// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "shell.h"
#include "gtest/gtest.h"

TEST(ShellTest, SimpleInitialization) {
  // The embedder must initialize the shell for us.
  ASSERT_TRUE(sky::shell::Shell::IsInitialized());
}
