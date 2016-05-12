// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "build/build_config.h"
#include "sky/test/test_suite.h"

#if defined(OS_MACOSX) && !defined(OS_IOS)
#include "sky/shell/platform/mac/platform_mac.h"
#endif

namespace flutter {
namespace test {

TestSuite::TestSuite(int argc, char** argv) : base::TestSuite(argc, argv) {}

TestSuite::~TestSuite() {}

void TestSuite::Initialize() {
#if defined(OS_MACOSX) && !defined(OS_IOS)
  sky::shell::PlatformMacMain(0, NULL, "");
#endif
}

void TestSuite::Shutdown() {}

}  // namespace test
}  // namespace flutter
