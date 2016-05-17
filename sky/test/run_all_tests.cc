// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/test/launcher/unit_test_launcher.h"

#include "test_suite.h"

int RunFlutterTestSuite(int argc, char** argv) {
  return flutter::test::TestSuite(argc, argv).Run();
}

int main(int argc, char** argv) {
  return base::LaunchUnitTestsSerially(
      argc, argv, base::Bind(&RunFlutterTestSuite, argc, argv));
}
