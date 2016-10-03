// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/platform/platform.h"

#include <stdio.h>

#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/window/window.h"
#include "lib/ftl/functional/wrap_lambda.h"
#include "lib/tonic/converter/dart_converter.h"
#include "lib/tonic/dart_library_natives.h"
#include "lib/tonic/logging/dart_invoke.h"

using tonic::DartInvoke;
using tonic::DartPersistentValue;
using tonic::DartState;

namespace blink {

namespace {

void PlatformServiceReturn(DartPersistentValue&& callback,
                           std::string return_data) {
  DartState* dart_state = callback.dart_state().get();
  if (!dart_state)
    return;
  DartState::Scope scope(dart_state);

  Dart_Handle return_data_handle =
      tonic::DartConverter<std::string>::ToDart(return_data);
  if (Dart_IsError(return_data_handle))
    return;

  DartInvoke(callback.Release(), {return_data_handle});
}

void PlatformService(Dart_NativeArguments args) {
  Dart_Handle exception = nullptr;

  std::string data =
      tonic::DartConverter<std::string>::FromArguments(args, 0, exception);
  if (exception) {
    Dart_ThrowException(exception);
    return;
  }

  Dart_Handle dart_callback =
      tonic::DartConverter<Dart_Handle>::FromArguments(args, 1, exception);
  if (exception) {
    Dart_ThrowException(exception);
    return;
  }
  DartPersistentValue callback(DartState::Current(), dart_callback);

  UIDartState::Current()->window()->client()->PlatformService(
      std::move(data), ftl::WrapLambda([callback = std::move(callback)](
                           std::string return_data) mutable {
        PlatformServiceReturn(std::move(callback), std::move(return_data));
      }));
}

}  // namespace

// static
void Platform::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register({
      {"_platformService", PlatformService, 2, true},
  });
}

}  // namespace blink
