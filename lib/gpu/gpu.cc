// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/gpu/gpu.h"
#include "flutter/fml/memory/ref_ptr.h"
#include "flutter/lib/ui/dart_wrapper.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/dart/runtime/include/dart_api.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_state.h"
#include "third_party/tonic/dart_wrappable.h"
#include "third_party/tonic/dart_wrapper_info.h"
#include "third_party/tonic/logging/dart_invoke.h"

namespace flutter {

IMPLEMENT_WRAPPERTYPEINFO(gpu, FlutterGpuTestClass);

}  // namespace flutter

uint32_t FlutterGpuTestProc() {
  return 1;
}

Dart_Handle FlutterGpuTestProcWithCallback(Dart_Handle callback) {
  flutter::UIDartState::ThrowIfUIOperationsProhibited();
  if (!Dart_IsClosure(callback)) {
    return tonic::ToDart("Callback must be a function");
  }

  tonic::DartInvoke(callback, {tonic::ToDart(1234)});

  return Dart_Null();
}

void FlutterGpuTestClass_Create(Dart_Handle wrapper) {
  auto res = fml::MakeRefCounted<flutter::FlutterGpuTestClass>();
  res->AssociateWithDartWrapper(wrapper);
  FML_LOG(ERROR) << "FlutterGpuTestClass Wrapped.";
}

void FlutterGpuTestClass_Method(flutter::FlutterGpuTestClass* self,
                                int something) {
  FML_LOG(ERROR) << "Something: " << something;
}
