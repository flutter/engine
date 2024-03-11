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

ProcTable::ProcTable()
    : lib_android_(fml::NativeLibrary::Create("libandroid.so")) {
  if (!lib_android_) {
    VALIDATION_LOG << "Could not open libandroid.so";
    return;
  }

  auto proc =
      lib_android_->ResolveFunction<decltype(&::android_get_device_api_level)>(
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

#define RESOLVE_PROC(table_member, api)                                   \
  {                                                                       \
    if (device_api_level_ >= table_member.api_availability) {             \
      if (auto resolved =                                                 \
              lib_android_->ResolveFunction<decltype(table_member.proc)>( \
                  table_member.proc_name);                                \
          resolved.has_value()) {                                         \
        table_member.proc = resolved.value();                             \
      } else {                                                            \
        VALIDATION_LOG << "Could not resolve function: "                  \
                       << table_member.proc_name;                         \
        return;                                                           \
      }                                                                   \
    }                                                                     \
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
