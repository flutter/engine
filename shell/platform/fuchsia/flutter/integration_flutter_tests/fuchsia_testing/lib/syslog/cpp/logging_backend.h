// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_SYSLOG_CPP_LOGGING_BACKEND_H_
#define LIB_SYSLOG_CPP_LOGGING_BACKEND_H_

#include <lib/syslog/cpp/log_settings.h>
#include <lib/syslog/cpp/macros.h>

#define WEAK __attribute__((weak))

namespace syslog_backend {

WEAK void SetLogSettings(const syslog::LogSettings& settings);

WEAK void SetLogSettings(const syslog::LogSettings& settings,
                         const std::initializer_list<std::string>& tags);

WEAK syslog::LogSeverity GetMinLogLevel();

}  // namespace syslog_backend

#endif  // LIB_SYSLOG_CPP_LOGGING_BACKEND_H_
