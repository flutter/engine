// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/run_configuration.h"
#include "flutter/runtime/dart_vm.h"

#include <sstream>

namespace shell {

RunConfiguration RunConfiguration::DefaultRunConfigurationFromSettings(
    const blink::Settings& settings) {
  // Precompiled code.
  if (blink::DartVM::IsRunningPrecompiledCode()) {
    return {
        .bundle_path = settings.flx_path,  //
        .entrypoint = "main",              //
        .kernel_snapshot = "",             //
        .script_snapshot = "",             //
        .packages_path = "",               //
        .main_path = "",                   //
    };
  }

  // Dart Sources.
  if (settings.main_dart_file_path.size() > 0) {
    return {
        .bundle_path = settings.flx_path,              //
        .entrypoint = "main",                          //
        .kernel_snapshot = "",                         //
        .script_snapshot = "",                         //
        .packages_path = settings.packages_file_path,  //
        .main_path = settings.main_dart_file_path,     //
    };
  }

  // Default.
  return {
      .bundle_path = settings.flx_path,              //
      .packages_path = settings.packages_file_path,  //
      .main_path = settings.main_dart_file_path,     //
  };
}

std::string RunConfiguration::ToString() const {
  auto string_or_default = [](const std::string& string,
                              const std::string& default_string) {
    return string.size() != 0 ? string : default_string;
  };

  std::stringstream stream;
  stream << std::endl;
  stream << "Bundle: " << string_or_default(bundle_path, "<Unspecified>")
         << std::endl;
  stream << "Entrypoint: " << entrypoint << std::endl;
  stream << "Kernel Snapshot: " << kernel_snapshot << std::endl;
  stream << "Script Snapshot: " << script_snapshot << std::endl;
  stream << "Packages Path: "
         << string_or_default(packages_path, "<Unspecified>") << std::endl;
  stream << "Main Path: " << string_or_default(main_path, "<Unspecified>")
         << std::endl;
  return stream.str();
}

}  // namespace shell
