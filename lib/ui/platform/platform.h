// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PLATFORM_PLATFORM_H_
#define FLUTTER_LIB_UI_PLATFORM_PLATFORM_H_

#include <functional>
#include <string>

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace blink {

class Platform {
 public:
  static void RegisterNatives(tonic::DartLibraryNatives* natives);
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_PLATFORM_PLATFORM_H_
