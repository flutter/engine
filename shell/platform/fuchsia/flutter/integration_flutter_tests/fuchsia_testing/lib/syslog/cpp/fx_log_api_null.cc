// Copyright 2021 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log_settings.h"
#include "macros.h"

namespace syslog_backend {
bool fx_log_compat_flush_record(LogBuffer* buffer) { return false; }

int fx_log_compat_reconfigure(syslog::LogSettings& settings,
                              const std::initializer_list<std::string>& tags) {
  return -1;
}
}  // namespace syslog_backend
