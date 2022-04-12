// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_PLATFORM_VIEW2_H_
#define FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_PLATFORM_VIEW2_H_

#include <flutter_plugin_registrar.h>

#include <map>
#include <memory>
#include <set>
#include <string>

#include "binary_messenger.h"
#include "texture_registrar.h"


namespace flutter {

// An extension to PluginRegistrar providing access to Windows-specific
// functionality.
class PlatformView2 {
 public:
  // Creates a new PlatformViewFactory. |core_registrar| and the messenger it
  // provides must remain valid as long as this object exists.
  explicit PlatformView2() {
  }

  virtual ~PlatformView2() {
  }

#ifndef WINUWP
  virtual void* GetHWND() = 0;
#endif

 private:
#ifndef WINUWP
#endif
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_COMMON_CLIENT_WRAPPER_INCLUDE_FLUTTER_PLATFORM_VIEW2_H_
