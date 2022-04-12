// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_FACTORY_WIN32_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_FACTORY_WIN32_H_

#include "flutter/shell/platform/common/client_wrapper/include/flutter/binary_messenger.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/encodable_value.h"
#include "flutter/shell/platform/common/client_wrapper/include/flutter/method_channel.h"
#include "flutter/shell/platform/windows/public/flutter_windows.h"
#include "flutter/shell/platform/windows/window_binding_handler.h"

#include "flutter/shell/platform/windows/platform_view_win32.h"

#include "encodable_value.h"
#include "message_codec.h"

namespace flutter {

// PlatformViewFactoryWin32
class PlatformViewFactoryWin32 {
 public:
  explicit PlatformViewFactoryWin32(
      flutter::MessageCodec<EncodableValue>& create_args_codec);
  virtual ~PlatformViewFactoryWin32();

  const flutter::MessageCodec<EncodableValue>& GetCreateArgsCodec();
  std::unique_ptr<PlatformViewWin32> Create();

 private:
  // The MethodChannel used for communication with the Flutter engine.
  const flutter::MessageCodec<EncodableValue>& create_args_codec_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_PLATFORM_VIEW_FACTORY_WIN32_H_
