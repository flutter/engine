// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/backtrace.h"

#include "flutter/fml/build_config.h"
#include "flutter/fml/paths.h"
#include "third_party/abseil-cpp/absl/debugging/failure_signal_handler.h"
#include "third_party/abseil-cpp/absl/debugging/symbolize.h"

namespace fml {

void InstallCrashHandler() {
#if FML_OS_WIN
  if (!IsDebuggerPresent()) {
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
  }
#endif
  auto exe_path = fml::paths::GetExecutablePath();
  if (exe_path.first) {
    absl::InitializeSymbolizer(exe_path.second.c_str());
  }
  absl::FailureSignalHandlerOptions options;
  absl::InstallFailureSignalHandler(options);
}

bool IsCrashHandlingSupported() {
  return true;
}

}  // namespace fml
