// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/tracing_controller.h"

#include <string>

#include "dart/runtime/include/dart_tools_api.h"
#include "flutter/common/threads.h"
#include "flutter/fml/trace_event.h"
#include "flutter/runtime/dart_init.h"
#include "flutter/shell/common/shell.h"
#include "lib/ftl/logging.h"

namespace shell {

TracingController::TracingController()
    : picture_tracing_enabled_(false), tracing_active_(false) {
  blink::SetEmbedderTracingCallbacks(
      std::unique_ptr<blink::EmbedderTracingCallbacks>(
          new blink::EmbedderTracingCallbacks([this]() { StartTracing(); },
                                              [this]() { StopTracing(); })));
}

TracingController::~TracingController() {
  blink::SetEmbedderTracingCallbacks(nullptr);
}

static void AddTraceMetadata() {
  blink::Threads::Gpu()->PostTask([]() { Dart_SetThreadName("gpu_thread"); });
  blink::Threads::UI()->PostTask([]() { Dart_SetThreadName("ui_thread"); });
  blink::Threads::IO()->PostTask([]() { Dart_SetThreadName("io_thread"); });
}

void TracingController::StartTracing() {
  if (tracing_active_)
    return;
  tracing_active_ = true;
  AddTraceMetadata();
}

void TracingController::StopTracing() {
  if (!tracing_active_) {
    return;
  }

  tracing_active_ = false;
}

std::string TracingController::TracePathWithExtension(
    const std::string& directory,
    const std::string& extension) const {
  base::Time::Exploded exploded;
  base::Time now = base::Time::Now();

  now.LocalExplode(&exploded);

  std::stringstream stream;
  // Example: trace_2015-10-08_at_11.38.25.121_.extension
  stream << directory << "/trace_" << exploded.year << "-" << exploded.month
         << "-" << exploded.day_of_month << "_at_" << exploded.hour << "."
         << exploded.minute << "." << exploded.second << "."
         << exploded.millisecond << "." << extension;
  return stream.str();
}

std::string TracingController::PictureTracingPathForCurrentTime() const {
  return PictureTracingPathForCurrentTime(traces_base_path_);
}

std::string TracingController::PictureTracingPathForCurrentTime(
    const std::string& directory) const {
  return TracePathWithExtension(directory, "skp");
}

}  // namespace shell
