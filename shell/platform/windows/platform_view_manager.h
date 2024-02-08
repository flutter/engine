// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_MANAGER_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_MANAGER_H_

#include <map>
#include <memory>

#include "flutter/shell/platform/common/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"

namespace flutter {

class PlatformViewManager {
 public:

 private:
  std::unique_ptr<MethodChannel<EncodableValue>> channel_;

  std::map<std::string, FlutterPlatformViewTypeEntry> platform_view_types_;

  std::map<int64_t, HWND> platform_views_;

  std::map<int64_t, FlutterPlatformViewCreationParameters> pending_platform_views_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_MANAGER_H_
