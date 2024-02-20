// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_ANDROID_PROC_TABLE_H_
#define FLUTTER_IMPELLER_TOOLKIT_ANDROID_PROC_TABLE_H_

#include <android/hardware_buffer.h>
#include <android/surface_control.h>

#include <functional>

#include "flutter/fml/native_library.h"

namespace impeller::android {

#define FOR_EACH_ANDROID_PROC(INVOKE)           \
  INVOKE(AHardwareBuffer_allocate, 26)          \
  INVOKE(AHardwareBuffer_acquire, 26)           \
  INVOKE(AHardwareBuffer_release, 26)           \
  INVOKE(AHardwareBuffer_isSupported, 29)       \
  INVOKE(AHardwareBuffer_describe, 26)          \
  INVOKE(ANativeWindow_acquire, 0)              \
  INVOKE(ANativeWindow_release, 0)              \
  INVOKE(ANativeWindow_getWidth, 0)             \
  INVOKE(ANativeWindow_getHeight, 0)            \
  INVOKE(ASurfaceControl_createFromWindow, 29)  \
  INVOKE(ASurfaceControl_release, 29)           \
  INVOKE(ASurfaceTransaction_create, 29)        \
  INVOKE(ASurfaceTransaction_delete, 29)        \
  INVOKE(ASurfaceTransaction_apply, 29)         \
  INVOKE(ASurfaceTransaction_setOnComplete, 29) \
  INVOKE(ASurfaceTransaction_reparent, 29)      \
  INVOKE(ASurfaceTransaction_setBuffer, 29)     \
  INVOKE(ASurfaceTransaction_setColor, 29)

template <class T>
struct AndroidProc {
  using AndroidProcType = T;

  const char* proc_name = nullptr;

  size_t api_availability = 0;

  AndroidProcType* proc = nullptr;

  template <class... Args>
  auto operator()(Args&&... args) const {
    return proc(std::forward<Args>(args)...);
  }
};

struct ProcTable {
#define DEFINE_PROC(name, api)                            \
  AndroidProc<decltype(name)> name = {.proc_name = #name, \
                                      .api_availability = api};
  FOR_EACH_ANDROID_PROC(DEFINE_PROC);
#undef DEFINE_PROC

  ProcTable();

  ~ProcTable();

  ProcTable(const ProcTable&) = delete;

  ProcTable& operator=(const ProcTable&) = delete;

  bool IsValid();

 private:
  fml::RefPtr<fml::NativeLibrary> lib_android_;
  bool is_valid_ = false;
};

const ProcTable& GetProcTable();

}  // namespace impeller::android

#endif  // FLUTTER_IMPELLER_TOOLKIT_ANDROID_PROC_TABLE_H_
