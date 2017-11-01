// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_COMMON_RUN_CONFIGURATION_H_
#define FLUTTER_SHELL_COMMON_RUN_CONFIGURATION_H_

#include <string>

#include "flutter/common/settings.h"
#include "flutter/runtime/dart_vm.h"
#include "lib/fxl/macros.h"

namespace shell {

struct RunConfiguration {
  std::string bundle_path;
  std::string entrypoint = "main";
  std::string kernel_snapshot = blink::DartVM::kKernelAssetKey;
  std::string script_snapshot = blink::DartVM::kSnapshotAssetKey;
  std::string packages_path;
  std::string main_path;

  static RunConfiguration DefaultRunConfigurationFromSettings(
      const blink::Settings& settings);

  std::string ToString() const;
};

}  // namespace shell

#endif  // FLUTTER_SHELL_COMMON_RUN_CONFIGURATION_H_
