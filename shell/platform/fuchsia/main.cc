// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/async-loop/cpp/loop.h>
#include <trace-provider/provider.h>
#include <trace/event.h>

#include <cstdlib>

#include "loop.h"
#include "runner.h"
#include "topaz/runtime/dart/utils/tempfs.h"

int main(int argc, char const* argv[]) {
  std::unique_ptr<async::Loop> loop(flutter_runner::MakeObservableLoop(true));

  fbl::unique_ptr<trace::TraceProvider> provider;
  {
    TRACE_DURATION("flutter", "CreateTraceProvider");
    bool already_started;
    // Use CreateSynchronously to prevent loss of early events.
    trace::TraceProvider::CreateSynchronously(
        loop->dispatcher(), "flutter_runner", &provider, &already_started);
  }

  // Set up the process-wide /tmp memfs.
  dart_utils::SetupRunnerTemp();

  FML_DLOG(INFO) << "Flutter application services initialized.";

  flutter_runner::Runner runner(loop.get());

  loop->Run();

  FML_DLOG(INFO) << "Flutter application services terminated.";

  return EXIT_SUCCESS;
}
