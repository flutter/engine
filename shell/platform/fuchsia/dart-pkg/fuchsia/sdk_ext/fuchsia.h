// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DART_PKG_FUCHSIA_SDK_EXT_FUCHSIA_H_
#define DART_PKG_FUCHSIA_SDK_EXT_FUCHSIA_H_

#include <fuchsia/sys/cpp/fidl.h>

namespace fuchsia {
namespace dart {

void Initialize(fidl::InterfaceHandle<fuchsia::sys::Environment> environment,
                zx::channel directory_request);

}  // namespace dart
}  // namespace fuchsia

#endif  // DART_PKG_FUCHSIA_SDK_EXT_FUCHSIA_H_
