// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "lib/syslog/cpp/log_settings.h"
#include "lib/syslog/global.h"
#include "logging_backend_shared.h"
#include "macros.h"

namespace syslog_backend {
bool fx_log_compat_flush_record(LogBuffer* buffer) {
  auto header = MsgHeader::CreatePtr(buffer);
  // Write fatal logs to stderr as well because death tests sometimes verify a certain
  // log message was printed prior to the crash.
  // TODO(samans): Convert tests to not depend on stderr. https://fxbug.dev/49593
  if (header->severity == syslog::LOG_FATAL) {
    std::cerr << header->c_str() << std::endl;
  }
  fx_logger_t* logger = fx_log_get_logger();
  if (header->user_tag) {
    fx_logger_log(logger, header->severity, header->user_tag, header->c_str());
  } else {
    fx_logger_log(logger, header->severity, nullptr, header->c_str());
  }
  return true;
}

int fx_log_compat_reconfigure(syslog::LogSettings& settings,
                              const std::initializer_list<std::string>& tags) {
  const char* ctags[FX_LOG_MAX_TAGS];
  size_t num_tags = 0;
  for (auto& tag : tags) {
    ctags[num_tags++] = tag.c_str();
    if (num_tags >= FX_LOG_MAX_TAGS)
      break;
  }
  int fd = -1;
  if (!settings.log_file.empty()) {
    fd = open(settings.log_file.c_str(), O_WRONLY | O_CREAT | O_APPEND);
    if (fd < 0) {
      fd = -1;
    }
  }
  fx_logger_config_t config = {.min_severity = settings.min_log_level,
                               .console_fd = fd,
                               .log_service_channel = ZX_HANDLE_INVALID,
                               .tags = ctags,
                               .num_tags = num_tags};
  fx_log_reconfigure(&config);
  return fd;
}

}  // namespace syslog_backend
