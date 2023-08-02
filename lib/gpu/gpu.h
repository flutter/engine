// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_GPU_GPU_H_
#define FLUTTER_LIB_GPU_GPU_H_

#include <cstdint>

#include "flutter/lib/ui/dart_wrapper.h"
#include "third_party/dart/runtime/include/dart_api.h"
#include "third_party/tonic/dart_state.h"
#include "third_party/tonic/dart_wrapper_info.h"

#if FML_OS_WIN
#define FLUTTER_EXPORT __declspec(dllexport)
#else  // FML_OS_WIN
#define FLUTTER_EXPORT __attribute__((visibility("default")))
#endif  // FML_OS_WIN

namespace flutter {

class FlutterGpuTestClass
    : public RefCountedDartWrappable<FlutterGpuTestClass> {
  DEFINE_WRAPPERTYPEINFO();

 public:
};

}  // namespace flutter

extern "C" {

FLUTTER_EXPORT
extern uint32_t FlutterGpuTestProc();

FLUTTER_EXPORT
extern Dart_Handle FlutterGpuTestProcWithCallback(Dart_Handle callback);

FLUTTER_EXPORT
extern void FlutterGpuTestClass_Create(Dart_Handle wrapper);

FLUTTER_EXPORT
extern void FlutterGpuTestClass_Method(flutter::FlutterGpuTestClass* self,
                                       int something);

}  // extern "C"

#endif  // FLUTTER_LIB_GPU_GPU_H_
