// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/engine/tonic/dart_io.h"

#include "dart/runtime/bin/io_natives.h"
#include "dart/runtime/include/dart_api.h"
#include "sky/engine/tonic/dart_converter.h"

namespace dart {
namespace bin {

extern bool capture_stdio;
extern bool capture_stdout;
extern bool capture_stderr;

}  // namespace bin
}  // namespace dart

namespace blink {

void DartIO::InitForIsolate() {
  DART_CHECK_VALID(Dart_SetNativeResolver(
      Dart_LookupLibrary(ToDart("dart:io")),
      dart::bin::IONativeLookup,
      dart::bin::IONativeSymbol));
}

}  // namespace blink
