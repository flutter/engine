// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/thread_host.h"

#include <memory>
#include <string>

namespace flutter {

ThreadHost::ThreadHost() = default;

ThreadHost::ThreadHost(ThreadHost&&) = default;

ThreadHost::ThreadHost(std::string name_prefix_arg,
                       uint64_t mask,
                       std::unique_ptr<ThreadConfigurer> configurer)
    : name_prefix(name_prefix_arg), thread_configurer(std::move(configurer)) {
  if (mask & ThreadHost::Type::Platform) {
    platform_thread = std::make_unique<fml::Thread>(name_prefix + ".platform");
  }

  if (mask & ThreadHost::Type::UI) {
    ui_thread = std::make_unique<fml::Thread>(name_prefix + ".ui", [&]() {
      if (thread_configurer) {
        thread_configurer->SetThreadPriority(fml::ThreadPriority::DISPLAY);
      }
    });
  }

  if (mask & ThreadHost::Type::RASTER) {
    raster_thread = std::make_unique<fml::Thread>(name_prefix + ".raster", [&] {
      if (thread_configurer) {
        thread_configurer->SetThreadPriority(fml::ThreadPriority::RASTER);
      }
    });
  }

  if (mask & ThreadHost::Type::IO) {
    io_thread = std::make_unique<fml::Thread>(name_prefix + ".io", [&] {
      if (thread_configurer) {
        thread_configurer->SetThreadPriority(fml::ThreadPriority::BACKGROUND);
      }
    });
  }

  if (mask & ThreadHost::Type::Profiler) {
    profiler_thread = std::make_unique<fml::Thread>(name_prefix + ".profiler");
  }
}

ThreadHost::~ThreadHost() = default;

}  // namespace flutter
