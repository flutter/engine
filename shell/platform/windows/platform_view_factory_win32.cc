// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/platform_view_factory_win32.h"

namespace flutter {

PlatformViewFactoryWin32::PlatformViewFactoryWin32(flutter::MessageCodec<EncodableValue>& create_args_codec)
    : create_args_codec_(create_args_codec) {
}

PlatformViewFactoryWin32::~PlatformViewFactoryWin32() = default;

const flutter::MessageCodec<EncodableValue>& PlatformViewFactoryWin32::GetCreateArgsCodec() {
  return create_args_codec_;
}

}  // namespace flutter
