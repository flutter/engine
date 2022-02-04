// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/thread_host.h"

#include <algorithm>
#include <memory>
#include <string>
#include <utility>

namespace flutter {

std::string ThreadHost::ThreadHostConfig::MakeThreadName(
    Type type,
    const std::string& prefix) {
  switch (type) {
    case Type::Platform:
      return prefix + ".platform";
    case Type::UI:
      return prefix + ".ui";
    case Type::IO:
      return prefix + ".io";
    case Type::RASTER:
      return prefix + ".raster";
    case Type::Profiler:
      return prefix + ".profiler";
  }
}

std::unique_ptr<fml::Thread> ThreadHost::CreateThread(
    Type type,
    ThreadHost::ThreadConfig configure) {
  std::string name = ThreadHostConfig::MakeThreadName(type, name_prefix);
  if (configure != nullptr) {
    return std::make_unique<fml::Thread>(std::move(configure));
  }
  return std::make_unique<fml::Thread>(
      fml::Thread::ThreadConfig::MakeDefaultConfigure(name));
}

ThreadHost::ThreadHost() = default;

ThreadHost::ThreadHost(ThreadHost&&) = default;

ThreadHost::ThreadHost(std::string name_prefix_arg,
                       uint64_t mask,
                       ThreadHostConfig configure_host)
    : name_prefix(name_prefix_arg) {
  if (mask & ThreadHost::Type::Platform) {
    platform_thread =
        CreateThread(ThreadHost::Type::Platform,
                     std::move(configure_host.platform_configure));
  }

  if (mask & ThreadHost::Type::UI) {
    ui_thread = CreateThread(ThreadHost::Type::UI,
                             std::move(configure_host.ui_configure));
  }

  if (mask & ThreadHost::Type::RASTER) {
    raster_thread = CreateThread(ThreadHost::Type::RASTER,
                                 std::move(configure_host.raster_configure));
  }

  if (mask & ThreadHost::Type::IO) {
    io_thread = CreateThread(ThreadHost::Type::IO,
                             std::move(configure_host.io_configure));
  }

  if (mask & ThreadHost::Type::Profiler) {
    profiler_thread =
        CreateThread(ThreadHost::Type::Profiler,
                     std::move(configure_host.profiler_configure));
    ;
  }
}

ThreadHost::~ThreadHost() = default;

}  // namespace flutter
