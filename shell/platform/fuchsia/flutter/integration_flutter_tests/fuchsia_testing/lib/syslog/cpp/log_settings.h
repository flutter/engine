// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_SYSLOG_CPP_LOG_SETTINGS_H_
#define LIB_SYSLOG_CPP_LOG_SETTINGS_H_

#include <lib/syslog/cpp/log_level.h>

#include <string>

namespace syslog {

// Settings which control the behavior of logging.
struct LogSettings {
  // The minimum logging level.
  // Anything at or above this level will be logged (if applicable).
  // Anything below this level will be silently ignored.
  //
  // The log level defaults to LOG_INFO.
  //
  // Log messages for FX_VLOGS(x) (from macros.h) log verbosities in
  // the range between INFO and DEBUG
  LogSeverity min_log_level = DefaultLogLevel;

  // The name of a file to which the log should be written.
  // When non-empty, the previous log output is closed and logging is
  // redirected to the specified file.  It is not possible to revert to
  // the previous log output through this interface.
  std::string log_file;

  // An fd to log to. Setting this will disable all structured
  // features resulting in plaintext-only logs written to the specified file descriptor.
  int log_fd = -1;
};

// Sets the active log settings for the current process.
void SetLogSettings(const LogSettings& settings);

// Sets the active log settings and tags for the current process. |tags| is not
// used on host.
void SetLogSettings(const LogSettings& settings, const std::initializer_list<std::string>& tags);

// Sets the log tags without modifying the settings. This is ignored on host.
void SetTags(const std::initializer_list<std::string>& tags);

// Gets the minimum log level for the current process. Never returns a value
// higher than LOG_FATAL.
LogSeverity GetMinLogLevel();

}  // namespace syslog

#endif  // LIB_SYSLOG_CPP_LOG_SETTINGS_H_
