// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>
#include <iostream>

#include "flutter/fml/build_config.h"
#include "flutter/fml/log_settings.h"
#include "flutter/fml/logging.h"

#if defined(OS_ANDROID)
#include <android/log.h>
#elif defined(OS_IOS)
#include <syslog.h>
#elif defined(OS_FUCHSIA)
#include <zircon/process.h>
#include <lib/syslog/logger.h>
#include <lib/syslog/structured_backend/cpp/fuchsia_syslog.h>
#include "lib/fdio/directory.h"
#include <fuchsia/logger/cpp/fidl.h>
#include <lib/async-loop/cpp/loop.h>
#include <lib/async-loop/default.h>
#include <lib/fidl/cpp/binding_set.h>
#endif

#if defined(OS_FUCHSIA)

static inline fx_log_severity_t fx_log_severity_from_verbosity(int verbosity) {
  if (verbosity < 0) {
    verbosity = 0;
  }
  // verbosity scale sits in the interstitial space between INFO and DEBUG
  int severity = FX_LOG_INFO - (verbosity * FX_LOG_VERBOSITY_STEP_SIZE);
  if (severity < FX_LOG_DEBUG + 1) {
    return FX_LOG_DEBUG + 1;
  }
  return severity;
}

zx_koid_t GetKoid(zx_handle_t handle) {
  zx_info_handle_basic_t info;
  zx_status_t status =
      zx_object_get_info(handle, ZX_INFO_HANDLE_BASIC, &info, sizeof(info), nullptr, nullptr);
  return status == ZX_OK ? info.koid : ZX_KOID_INVALID;
}

class FuchsiaState {
 public:
  FuchsiaState() {
    ConnectAsync();
  }
  void BeginRecord(fuchsia_syslog::LogBuffer* buffer, FuchsiaLogSeverity severity,
                   cpp17::optional<cpp17::string_view> file_name, unsigned int line,
                   cpp17::optional<cpp17::string_view> msg,
                   cpp17::optional<cpp17::string_view> condition) {
    buffer->BeginRecord(severity, file_name, line, msg, condition, false,
                                socket_.borrow(), 0, GetKoid(zx_process_self()),
                                GetKoid(zx_thread_self()));
  }
  void WriteLog(int severity,
                       const char* file,
                       int line,
                       const char* msg) {
                         fuchsia_syslog::LogBuffer buffer;
                         BeginRecord(&buffer, severity, file, line, msg, cpp17::nullopt);
                         buffer.FlushRecord();
                       }
  private:
  void ConnectAsync() {
    zx::channel logger, logger_request;
    if (zx::channel::create(0, &logger, &logger_request) != ZX_OK) {
      return;
    }
    // TODO(https://fxbug.dev/75214): Support for custom names.
    if (fdio_service_connect("/svc/fuchsia.logger.LogSink", logger_request.release()) != ZX_OK) {
      return;
    }
    if (log_sink_.Bind(std::move(logger)) != ZX_OK) {
      return;
    }
    zx::socket local, remote;
    if (zx::socket::create(ZX_SOCKET_DATAGRAM, &local, &remote) != ZX_OK) {
      return;
    }
    log_sink_->ConnectStructured(std::move(remote));
    socket_ = std::move(local);
  }
  zx::socket socket_;
  fuchsia::logger::LogSinkPtr log_sink_;
};
FuchsiaState* g_fuchsia_logger = nullptr;

void init_fuchsia_logging() {
  g_fuchsia_logger = new FuchsiaState();
}
#endif

namespace fml {

namespace {

#if !defined(OS_FUCHSIA)
const char* const kLogSeverityNames[LOG_NUM_SEVERITIES] = {"INFO", "WARNING",
                                                           "ERROR", "FATAL"};

const char* GetNameForLogSeverity(LogSeverity severity) {
  if (severity >= LOG_INFO && severity < LOG_NUM_SEVERITIES) {
    return kLogSeverityNames[severity];
  }
  return "UNKNOWN";
}

const char* StripDots(const char* path) {
  while (strncmp(path, "../", 3) == 0) {
    path += 3;
  }
  return path;
}

const char* StripPath(const char* path) {
  auto* p = strrchr(path, '/');
  if (p) {
    return p + 1;
  }
  return path;
}
#endif

}  // namespace

LogMessage::LogMessage(LogSeverity severity,
                       const char* file,
                       int line,
                       const char* condition)
    : severity_(severity), file_(file), line_(line) {
#if !defined(OS_FUCHSIA)
  stream_ << "[";
  if (severity >= LOG_INFO) {
    stream_ << GetNameForLogSeverity(severity);
  } else {
    stream_ << "VERBOSE" << -severity;
  }
  stream_ << ":" << (severity > LOG_INFO ? StripDots(file_) : StripPath(file_))
          << "(" << line_ << ")] ";
#endif

  if (condition) {
    stream_ << "Check failed: " << condition << ". ";
  }
}

LogMessage::~LogMessage() {
#if !defined(OS_FUCHSIA)
  stream_ << std::endl;
#endif

#if defined(OS_ANDROID)
  android_LogPriority priority =
      (severity_ < 0) ? ANDROID_LOG_VERBOSE : ANDROID_LOG_UNKNOWN;
  switch (severity_) {
    case LOG_INFO:
      priority = ANDROID_LOG_INFO;
      break;
    case LOG_WARNING:
      priority = ANDROID_LOG_WARN;
      break;
    case LOG_ERROR:
      priority = ANDROID_LOG_ERROR;
      break;
    case LOG_FATAL:
      priority = ANDROID_LOG_FATAL;
      break;
  }
  __android_log_write(priority, "flutter", stream_.str().c_str());
#elif defined(OS_IOS)
  syslog(LOG_ALERT, "%s", stream_.str().c_str());
#elif defined(OS_FUCHSIA)
  FuchsiaLogSeverity fx_severity;
  switch (severity_) {
    case LOG_INFO:
      fx_severity = FUCHSIA_LOG_INFO;
      break;
    case LOG_WARNING:
      fx_severity = FUCHSIA_LOG_WARNING;
      break;
    case LOG_ERROR:
      fx_severity = FUCHSIA_LOG_ERROR;
      break;
    case LOG_FATAL:
      fx_severity = FUCHSIA_LOG_FATAL;
      break;
    default:
      if (severity_ < 0) {
        fx_severity = fx_log_severity_from_verbosity(-severity_);
      } else {
        // Unknown severity. Use INFO.
        fx_severity = FUCHSIA_LOG_INFO;
      }
  }
  g_fuchsia_logger->WriteLog(fx_severity, file_, line_, stream_.str().c_str());
#else
  std::cerr << stream_.str();
  std::cerr.flush();
#endif

  if (severity_ >= LOG_FATAL) {
    KillProcess();
  }
}

int GetVlogVerbosity() {
  return std::max(-1, LOG_INFO - GetMinLogLevel());
}

bool ShouldCreateLogMessage(LogSeverity severity) {
  return severity >= GetMinLogLevel();
}

void KillProcess() {
  abort();
}

}  // namespace fml
