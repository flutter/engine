// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_SYSLOG_CPP_LOG_LEVEL_H_
#define LIB_SYSLOG_CPP_LOG_LEVEL_H_

#include <cstdint>

namespace syslog {

typedef int8_t LogSeverity;

// Default log levels.
constexpr LogSeverity LOG_TRACE = 0x10;
constexpr LogSeverity LOG_DEBUG = 0x20;
constexpr LogSeverity LOG_INFO = 0x30;
constexpr LogSeverity LOG_WARNING = 0x40;
constexpr LogSeverity LOG_ERROR = 0x50;
constexpr LogSeverity LOG_FATAL = 0x60;

constexpr LogSeverity LOG_NONE = 0x00;

constexpr LogSeverity DefaultLogLevel = LOG_INFO;
constexpr int LogSeverityStepSize = 0x10;
constexpr int LogVerbosityStepSize = 0x1;

// LOG_DFATAL is LOG_FATAL in debug mode, LOG_ERROR in normal mode
#ifdef NDEBUG
const LogSeverity LOG_DFATAL = LOG_ERROR;
#else
const LogSeverity LOG_DFATAL = LOG_FATAL;
#endif

inline LogSeverity LOG_LEVEL(int8_t level) { return level; }

}  // namespace syslog

#endif  // LIB_SYSLOG_CPP_LOG_LEVEL_H_
