// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/testing/testing.h"

#include "flutter/shell/common/switches.h"
#include "flutter/shell/testing/test_runner.h"

namespace shell {

bool InitForTesting(const fxl::CommandLine& command_line) {
  RunConfiguration config;
  config.packages_path = command_line.GetOptionValueWithDefault(
      FlagForSwitch(Switch::Packages), "");
  auto args = command_line.positional_args();
  if (args.empty()) {
    FXL_LOG(INFO) << "Main dart file path not specified.";
    return false;
  }
  config.main_path = args[0];
  TestRunner::Shared().Run(std::move(config));
  return true;
}

}  // namespace shell
