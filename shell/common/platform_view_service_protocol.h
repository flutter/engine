// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_COMMON_VIEW_SERVICE_PROTOCOL_H_
#define SHELL_COMMON_VIEW_SERVICE_PROTOCOL_H_

#include <memory>

#include "flutter/shell/common/platform_view.h"
#include "lib/fxl/synchronization/waitable_event.h"
#include "third_party/dart/runtime/include/dart_tools_api.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace shell {

class PlatformViewServiceProtocol {
 public:
  static void RegisterHook(Shell* shell);
};

}  // namespace shell

#endif  // SHELL_COMMON_VIEW_SERVICE_PROTOCOL_H_
