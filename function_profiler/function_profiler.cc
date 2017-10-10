// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#if !__has_attribute(no_instrument_function)

#error "Function profiling must be available."

#endif  // !__has_attribute(no_instrument_function)

// We don't need the macros since they only help in adding scoped traces which
// we wont be able to use.
#define TRACE_EVENT_HIDE_MACROS

#include "flutter/function_profiler/function_profiler.h"
#include <dlfcn.h>
#include "flutter/fml/trace_event.h"

static const char* kProfilerCategory = "func-profiler";

#define FUNC_ENTER __cyg_profile_func_enter
#define FUNC_EXIT __cyg_profile_func_exit

extern "C" {
// NO_INSTRUMENT_FUNCTION only present to prevent cases where the user
// accidently decides to profile the profiler.
static const char* GetSymbolName(const void* addr) NO_INSTRUMENT_FUNCTION;
void FUNC_ENTER(void* this_fn, void* call_site) NO_INSTRUMENT_FUNCTION;
void FUNC_EXIT(void* this_fn, void* call_site) NO_INSTRUMENT_FUNCTION;
}

static const char* GetSymbolName(const void* addr) {
  Dl_info info;
  if (dladdr(addr, &info) == 0) {
    return nullptr;
  }
  return info.dli_sname;
}

void FUNC_ENTER(void* this_fn, void* call_site) {
  if (auto name = GetSymbolName(call_site)) {
    fml::tracing::TraceEvent0(kProfilerCategory, name);
  }
}

void FUNC_EXIT(void* this_fn, void* call_site) {
  if (auto name = GetSymbolName(call_site)) {
    fml::tracing::TraceEventEnd(name);
  }
}
