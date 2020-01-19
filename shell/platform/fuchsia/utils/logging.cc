// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/fuchsia/utils/logging.h"

#include <algorithm>  // For std::max
#include <cstdlib>    // For abort()
#include <iostream>   // For std::cerr

namespace fx {
namespace {

const char* const kLogSeverityNames[LOG_NUM_SEVERITIES] = {"INFO", "WARNING",
                                                           "ERROR", "FATAL"};

const char* GetNameForLogSeverity(LogSeverity severity) {
  if (severity >= LOG_INFO && severity < LOG_NUM_SEVERITIES)
    return kLogSeverityNames[severity];
  return "UNKNOWN";
}

const char* StripDots(const char* path) {
  while (strncmp(path, "../", 3) == 0)
    path += 3;
  return path;
}

const char* StripPath(const char* path) {
  auto* p = strrchr(path, '/');
  if (p)
    return p + 1;
  else
    return path;
}

}  // namespace

LogMessage::LogMessage(LogSeverity severity,
                       const char* file,
                       int line,
                       const char* condition)
    : severity_(severity), file_(file), line_(line) {
  stream_ << "[";
  if (severity >= LOG_INFO)
    stream_ << GetNameForLogSeverity(severity);
  else
    stream_ << "VERBOSE" << -severity;
  stream_ << ":" << (severity > LOG_INFO ? StripDots(file_) : StripPath(file_))
          << "(" << line_ << ")] ";

  if (condition)
    stream_ << "Check failed: " << condition << ". ";
}

LogMessage::~LogMessage() {
  stream_ << std::endl;

  std::cerr << stream_.str();
  std::cerr.flush();

  if (severity_ >= LOG_FATAL) {
    abort();
  }
}

int GetVlogVerbosity() {
  return std::max(-1, LOG_INFO - GetMinLogLevel());
}

bool ShouldCreateLogMessage(LogSeverity severity) {
  return severity >= GetMinLogLevel();
}

}  // namespace fx
