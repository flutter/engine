// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_SYSLOG_CPP_LOGGING_BACKEND_FUCHSIA_GLOBALS_H_
#define LIB_SYSLOG_CPP_LOGGING_BACKEND_FUCHSIA_GLOBALS_H_

#include <zircon/types.h>

#include <cstdint>

namespace syslog_backend {
class LogState;
}  // namespace syslog_backend

extern "C" {

syslog_backend::LogState* SetState(syslog_backend::LogState* new_state);

syslog_backend::LogState* GetState();

uint32_t GetAndResetDropped();

void AddDropped(uint32_t count);

zx_koid_t GetCurrentThreadKoid();

}  // extern "C"

#endif  // LIB_SYSLOG_CPP_LOGGING_BACKEND_FUCHSIA_GLOBALS_H_
