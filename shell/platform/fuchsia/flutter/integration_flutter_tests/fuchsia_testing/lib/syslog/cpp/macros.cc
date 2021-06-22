// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <lib/syslog/cpp/log_settings.h>
#include <lib/syslog/cpp/logging_backend.h>
#include <lib/syslog/cpp/macros.h>

#include <algorithm>
#include <cstring>
#include <iostream>
#include <memory>

#ifdef __Fuchsia__
#include <zircon/status.h>
#endif

namespace syslog {
namespace {

const char* StripDots(const char* path) {
  while (strncmp(path, "../", 3) == 0)
    path += 3;
  return path;
}

const char* StripPath(const char* path) {
  auto p = strrchr(path, '/');
  if (p)
    return p + 1;
  else
    return path;
}

}  // namespace

LogMessage::LogMessage(LogSeverityAndId severity_and_id, const char* file, int line,
                       const char* condition, const char* tag
#if defined(__Fuchsia__)
                       ,
                       zx_status_t status
#endif
                       )
    : severity_(severity_and_id.severity()),
      log_id_(severity_and_id.id()),
      file_(severity_ > LOG_INFO ? StripDots(file) : StripPath(file)),
      line_(line),
      condition_(condition),
      tag_(tag)
#if defined(__Fuchsia__)
      ,
      status_(status)
#endif
{
}

LogMessage::LogMessage(LogSeverity severity, const char* file, int line, const char* condition,
                       const char* tag
#if defined(__Fuchsia__)
                       ,
                       zx_status_t status
#endif
                       )
    : LogMessage(LogSeverityAndId(severity), file, line, condition, tag
#if defined(__Fuchsia__)
                 ,
                 status
#endif
      ) {
}

LogMessage::~LogMessage() {
#if defined(__Fuchsia__)
  if (status_ != std::numeric_limits<zx_status_t>::max()) {
    stream_ << ": " << status_ << " (" << zx_status_get_string(status_) << ")";
  }
#endif
  auto buffer = std::make_unique<syslog_backend::LogBuffer>();
  auto str = stream_.str();
  syslog_backend::BeginRecord(buffer.get(), severity_, file_, line_, str.data(), condition_);
  if (tag_) {
    syslog_backend::WriteKeyValue(buffer.get(), "tag", tag_);
  }
  if (log_id_) {
    syslog_backend::WriteKeyValue(buffer.get(), "log_id", log_id_);
  }
  syslog_backend::EndRecord(buffer.get());
  syslog_backend::FlushRecord(buffer.get());
  if (severity_ >= LOG_FATAL)
    __builtin_debugtrap();
}

bool LogFirstNState::ShouldLog(uint32_t n) {
  const uint32_t counter_value = counter_.fetch_add(1, std::memory_order_relaxed);
  return counter_value < n;
}

int GetVlogVerbosity() {
  int min_level = GetMinLogLevel();
  if (min_level < LOG_INFO && min_level > LOG_DEBUG) {
    return LOG_INFO - min_level;
  }
  return 0;
}

bool ShouldCreateLogMessage(LogSeverity severity) { return severity >= GetMinLogLevel(); }

bool ShouldCreateLogMessage(const LogSeverityAndId& severity_and_id) {
  return severity_and_id.severity() >= GetMinLogLevel();
}

}  // namespace syslog
