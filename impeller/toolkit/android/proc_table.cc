// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/toolkit/android/proc_table.h"

#include "impeller/base/validation.h"

namespace impeller::android {

const ProcTable& GetProcTable() {
  static ProcTable gProcTable;
  return gProcTable;
}

template <class T>
bool ResolveAndroidProc(
    AndroidProc<T>& proc,
    uint32_t device_api_level,
    const std::vector<fml::RefPtr<fml::NativeLibrary>>& libs) {
  if (device_api_level < proc.api_availability) {
    // Nothing to do. We don't expect this proc. to be available on this device.
    return true;
  }
  std::optional<T*> proc_value;
  for (const auto& lib : libs) {
    proc_value = lib->ResolveFunction<T*>(proc.proc_name);
    if (proc_value) {
      break;
    }
  }
  if (!proc_value.has_value()) {
    VALIDATION_LOG << "Could not find proc in any of the Android libraries: "
                   << proc.proc_name;
    return false;
  }
  proc.proc = proc_value.value();
  return true;
}

ProcTable::ProcTable() {
  auto lib_android = fml::NativeLibrary::Create("libandroid.so");
  auto lib_egl = fml::NativeLibrary::Create("libEGL.so");

  if (!lib_android || !lib_egl) {
    VALIDATION_LOG << "Could not open Android libraries.";
    return;
  }

  libraries_.push_back(lib_android);
  libraries_.push_back(lib_egl);

  auto proc =
      lib_android->ResolveFunction<decltype(&::android_get_device_api_level)>(
          "android_get_device_api_level");
  if (!proc.has_value()) {
    return;
  }

  int api = proc.value()();
  if (api < 0) {
    VALIDATION_LOG << "Could not get Android API level.";
    return;
  }

  device_api_level_ = api;

#define RESOLVE_PROC(table_member, api)                                     \
  {                                                                         \
    if (!ResolveAndroidProc(table_member, device_api_level_, libraries_)) { \
      return;                                                               \
    }                                                                       \
  }
  FOR_EACH_ANDROID_PROC(RESOLVE_PROC);
#undef RESOLVE_PROC

  is_valid_ = true;
}

ProcTable::~ProcTable() = default;

bool ProcTable::IsValid() const {
  return is_valid_;
}

uint32_t ProcTable::GetAndroidDeviceAPILevel() const {
  return device_api_level_;
}

}  // namespace impeller::android
