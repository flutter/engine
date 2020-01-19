// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_LOGGING_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_LOGGING_H_

#include <sstream>

namespace fx {

typedef int LogSeverity;

// Default log levels. Negative values can be used for verbose log levels.
constexpr LogSeverity LOG_INFO = 0;
constexpr LogSeverity LOG_WARNING = 1;
constexpr LogSeverity LOG_ERROR = 2;
constexpr LogSeverity LOG_FATAL = 3;
constexpr LogSeverity LOG_NUM_SEVERITIES = 4;

// LOG_DFATAL is LOG_FATAL in debug mode, ERROR in normal mode
#ifdef NDEBUG
const LogSeverity LOG_DFATAL = LOG_ERROR;
#else
const LogSeverity LOG_DFATAL = LOG_FATAL;
#endif

class LogMessageVoidify {
 public:
  void operator&(std::ostream&) {}
};

class LogMessage {
 public:
  LogMessage(LogSeverity severity,
             const char* file,
             int line,
             const char* condition);
  LogMessage(const LogMessage&) = delete;
  LogMessage(LogMessage&&) = delete;
  ~LogMessage();

  LogMessage& operator=(const LogMessage&) = delete;
  LogMessage& operator=(LogMessage&&) = delete;

  std::ostream& stream() { return stream_; }

 private:
  std::ostringstream stream_;
  const LogSeverity severity_;
  const char* file_;
  const int line_;
};

// TODO(dworsham): Re-add log_tag and use syslog directly?

// Gets the minimum log level for the current process. Never returs a value
// higher than LOG_FATAL.
inline int GetMinLogLevel() {
  // TODO(dworsham): verbose logging like in shell.cc?
  return LOG_ERROR;
}

// Gets the FX_VLOG default verbosity level.
int GetVlogVerbosity();

// Returns true if |severity| is at or above the current minimum log level.
// LOG_FATAL and above is always true.
bool ShouldCreateLogMessage(LogSeverity severity);

}  // namespace fx

#define FX_LOG_STREAM(severity) \
  ::fx::LogMessage(::fx::LOG_##severity, __FILE__, __LINE__, nullptr).stream()

#define FX_LAZY_STREAM(stream, condition) \
  !(condition) ? (void)0 : ::fx::LogMessageVoidify() & (stream)

#define FX_EAT_STREAM_PARAMETERS(ignored) \
  true || (ignored)                       \
      ? (void)0                           \
      : ::fx::LogMessageVoidify() &       \
            ::fx::LogMessage(::fx::LOG_FATAL, 0, 0, nullptr).stream()

#define FX_LOG_IS_ON(severity) \
  (::fx::ShouldCreateLogMessage(::fx::LOG_##severity))

#define FX_LOG(severity) \
  FX_LAZY_STREAM(FX_LOG_STREAM(severity), FX_LOG_IS_ON(severity))

#define FX_CHECK(condition)                                             \
  FX_LAZY_STREAM(                                                       \
      ::fx::LogMessage(::fx::LOG_FATAL, __FILE__, __LINE__, #condition) \
          .stream(),                                                    \
      !(condition))

#define FX_VLOG_IS_ON(verbose_level) \
  ((verbose_level) <= ::fx::GetVlogVerbosity())

// The VLOG macros log with negative verbosities.
#define FX_VLOG_STREAM(verbose_level) \
  ::fx::LogMessage(-verbose_level, __FILE__, __LINE__, nullptr).stream()

#define FX_VLOG(verbose_level) \
  FX_LAZY_STREAM(FX_VLOG_STREAM(verbose_level), FX_VLOG_IS_ON(verbose_level))

#ifndef NDEBUG
#define FX_DLOG(severity) FX_LOG(severity)
#define FX_DCHECK(condition) FX_CHECK(condition)
#else
#define FX_DLOG(severity) FX_EAT_STREAM_PARAMETERS(true)
#define FX_DCHECK(condition) FX_EAT_STREAM_PARAMETERS(condition)
#endif

#define FX_NOTREACHED() FX_DCHECK(false)

#define FX_NOTIMPLEMENTED() \
  FX_LOG(ERROR) << "Not implemented in: " << __PRETTY_FUNCTION__

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_UTILS_LOGGING_H_
