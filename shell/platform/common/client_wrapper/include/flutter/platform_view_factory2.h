// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_PLATFORM_VIEW_FACTORY2_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_PLATFORM_VIEW_FACTORY2_H_

#include <windows.h>

#include <memory>
#include <optional>

#include "platform_view2.h"
#include "plugin_registrar.h"

namespace flutter {

// An extension to PluginRegistrar providing access to Windows-specific
// functionality.
class PlatformViewFactory2 {
 public:
  // Creates a new PlatformViewFactory. |core_registrar| and the messenger it
  // provides must remain valid as long as this object exists.
  explicit PlatformViewFactory2() {
  }

  virtual ~PlatformViewFactory2() = default;

  virtual std::unique_ptr<PlatformView2> Create(void* user_data) = 0;

 private:
#ifndef WINUWP
#endif
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_PLATFORM_VIEW_FACTORY2_H_
