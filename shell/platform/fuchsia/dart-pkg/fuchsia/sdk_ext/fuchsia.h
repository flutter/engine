// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_FUCHSIA_SDK_EXT_FUCHSIA_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_FUCHSIA_SDK_EXT_FUCHSIA_H_

#include <fuchsia/io/cpp/fidl.h>
#include <fuchsia/sys/cpp/fidl.h>
#include <lib/fidl/cpp/interface_handle.h>
#include <lib/fidl/cpp/interface_request.h>
#include <lib/zx/channel.h>
#include <lib/zx/event.h>

#include <optional>

namespace fuchsia::dart {

void Initialize(
    fidl::InterfaceHandle<fuchsia::sys::Environment> environment,
    fidl::InterfaceRequest<fuchsia::io::Directory> directory_request);

}  // namespace fuchsia::dart

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_DART_PKG_FUCHSIA_SDK_EXT_FUCHSIA_H_
