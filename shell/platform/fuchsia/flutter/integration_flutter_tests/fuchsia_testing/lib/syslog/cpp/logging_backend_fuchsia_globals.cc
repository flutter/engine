// Copyright 2020 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sdk/lib/syslog/cpp/logging_backend_fuchsia_globals.h"

#include <zircon/assert.h>
#include <zircon/process.h>
#include <zircon/syscalls.h>

#include <atomic>

#define EXPORT __attribute__((visibility("default")))

namespace {

std::atomic<uint32_t> dropped_count = std::atomic<uint32_t>(0);
std::atomic<syslog_backend::LogState*> state = nullptr;

// This thread's koid.
// Initialized on first use.
thread_local zx_koid_t tls_thread_koid{ZX_KOID_INVALID};

zx_koid_t GetKoid(zx_handle_t handle) {
  zx_info_handle_basic_t info;
  zx_status_t status =
      zx_object_get_info(handle, ZX_INFO_HANDLE_BASIC, &info, sizeof(info), nullptr, nullptr);
  return status == ZX_OK ? info.koid : ZX_KOID_INVALID;
}

}  // namespace

extern "C" {

EXPORT
zx_koid_t GetCurrentThreadKoid() {
  if (unlikely(tls_thread_koid == ZX_KOID_INVALID)) {
    tls_thread_koid = GetKoid(zx_thread_self());
  }
  ZX_DEBUG_ASSERT(tls_thread_koid != ZX_KOID_INVALID);
  return tls_thread_koid;
}

EXPORT
syslog_backend::LogState* SetState(syslog_backend::LogState* new_state) {
  return state.exchange(new_state);
}

EXPORT
syslog_backend::LogState* GetState() { return state.load(); }

EXPORT
uint32_t GetAndResetDropped() { return dropped_count.exchange(0, std::memory_order_relaxed); }

EXPORT
void AddDropped(uint32_t count) { dropped_count.fetch_add(count, std::memory_order_relaxed); }

}  // extern "C"
