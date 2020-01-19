// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/async-loop/cpp/loop.h>
#include <lib/async-loop/default.h>
#include <lib/trace/event.h>

#include <cstdlib>  // For EXIT_SUCCESS

#include "flutter/shell/platform/fuchsia/dart_runner/dart_runner.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/shell/platform/fuchsia/utils/tempfs.h"

#if !defined(DART_PRODUCT)
#include <lib/trace-provider/provider.h>

#include "flutter/shell/platform/fuchsia/utils/files.h"
#include "third_party/dart/runtime/include/dart_api.h"
#endif  // !defined(DART_PRODUCT)

#if !defined(DART_PRODUCT)
// Register native symbol information for the Dart VM's profiler.
static void RegisterProfilerSymbols(const char* symbols_path,
                                    const char* dso_name) {
  std::string* symbols = new std::string();
  if (fx::ReadFileToString(symbols_path, symbols)) {
    Dart_AddSymbols(dso_name, symbols->data(), symbols->size());
  } else {
    FX_LOG(ERROR) << "Failed to load " << symbols_path;
    FX_CHECK(false);
  }
}
#endif  // !defined(DART_PRODUCT)

int main(int argc, const char** argv) {
  async::Loop loop(&kAsyncLoopConfigAttachToCurrentThread);

  // TODO(dworsham): Use syslog directly?
  // Setup logging to the syslog.
  // fx_log_init();

#if !defined(DART_PRODUCT)
  // Set up tracing as early as possible to capture all events.
  std::unique_ptr<trace::TraceProviderWithFdio> provider;
  {
    bool already_started;
    // Use CreateSynchronously to prevent loss of early events.
    trace::TraceProviderWithFdio::CreateSynchronously(
        loop.dispatcher(), "dart_runner", &provider, &already_started);
  }

#if defined(AOT_RUNTIME)
  RegisterProfilerSymbols("pkg/data/dart_aot_runner.dartprofilersymbols", "");
#else
  RegisterProfilerSymbols("pkg/data/dart_jit_runner.dartprofilersymbols", "");
#endif  // defined(AOT_RUNTIME)
#endif  // !defined(DART_PRODUCT)

  fx::RunnerTemp runner_temp;  // Set up the process-wide /tmp memfs.
  dart_runner::DartRunner runner;

  // Run the Runner. :)
  loop.Run();

  return EXIT_SUCCESS;
}
