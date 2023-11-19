// Copyright 2023 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_WINDOW_PLATFORM_ISOLATE_H_
#define FLUTTER_LIB_UI_WINDOW_PLATFORM_ISOLATE_H_

namespace flutter {

class PlatformIsolateNativeApi {
 public:
  static Dart_Handle Spawn(Dart_Handle entry_point, Dart_Handle message);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_WINDOW_PLATFORM_ISOLATE_H_
