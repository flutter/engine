// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/async-loop/cpp/loop.h>
#include <lib/async-loop/default.h>
#include <lib/trace/event.h>

#include <cstdlib>  // For EXIT_SUCCESS
#include <memory>

#include "flutter/shell/platform/fuchsia/flutter/renderer.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_window.h"
#include "flutter/shell/platform/fuchsia/flutter/runner.h"
#include "flutter/shell/platform/fuchsia/utils/logging.h"
#include "flutter/shell/platform/fuchsia/utils/tempfs.h"

#if !defined(DART_PRODUCT)
#include <lib/trace-provider/provider.h>
#endif  // !defined(DART_PRODUCT)

int main(int argc, char const* argv[]) {
  async::Loop loop(&kAsyncLoopConfigAttachToCurrentThread);

  // TODO(dworsham): Use syslog directly for logging?
  // Setup logging to the syslog.
  // fx_log_init();

#if !defined(DART_PRODUCT)
  // Set up tracing as early as possible to capture all events.
  std::unique_ptr<trace::TraceProviderWithFdio> provider;
  {
    bool already_started;
    // Use CreateSynchronously to prevent loss of early events.
    bool trace_created = trace::TraceProviderWithFdio::CreateSynchronously(
        loop.dispatcher(), "flutter_runner", &provider, &already_started);
    FX_DCHECK(trace_created);
  }
#endif  // !defined(DART_PRODUCT)

  fx::RunnerTemp runner_temp;  // Set up the process-wide /tmp memfs.
  flutter_runner::Runner runner(
      [](flutter_runner::Renderer::Context renderer_context)
          -> std::unique_ptr<flutter_runner::Renderer> {
        return std::make_unique<flutter_runner::ScenicWindow>(
            std::move(renderer_context));
      },
      &loop);

  // Run the Runner. :)
  loop.Run();

  return EXIT_SUCCESS;
}
