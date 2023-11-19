// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

namespace flutter {

Dart_Handle PlatformIsolateNativeApi::Spawn(
    Dart_Handle entry_point, Dart_Handle message) {
  DartIsolate::CreateRunningPlatformIsolate();
}

}  // namespace flutter
