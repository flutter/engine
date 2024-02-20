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

#define RESOLVE_PROC(table_member, api)                             \
  {                                                                 \
    auto resolved =                                                 \
        lib_android_->ResolveFunction<decltype(table_member.proc)>( \
            table_member.proc_name);                                \
    if (!resolved.has_value()) {                                    \
      VALIDATION_LOG << "Could not resolve function: "              \
                     << table_member.proc_name;                     \
      return;                                                       \
    }                                                               \
    table_member.proc = resolved.value();                           \
  }
  FOR_EACH_ANDROID_PROC(RESOLVE_PROC);
#undef RESOLVE_PROC
  is_valid_ = true;
}

ProcTable::~ProcTable() = default;

bool ProcTable::IsValid() {
  return is_valid_;
}

}  // namespace impeller::android
