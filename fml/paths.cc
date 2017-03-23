// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/paths.h"

#include <mutex>
#include <thread>

#include "lib/ftl/build_config.h"

#if OS_LINUX

#include "flutter/fml/platform/linux/path_provider_linux.h"
using PlatformPathProvider = fml::paths::PathProviderLinux;

#elif OS_ANDROID

#include "flutter/fml/platform/android/path_provider_android.h"
using PlatformPathProvider = fml::paths::PathProviderAndroid;

#elif OS_MACOSX

#include "flutter/fml/platform/darwin/path_provider_darwin.h"
using PlatformPathProvider = fml::paths::PathProviderDarwin;

#else

#error This platform does not have a path provider implementation.

#endif

namespace fml {
namespace paths {

std::pair<bool, std::string> GetPath(PathType type) {
  static std::once_flag once;
  static PathProvider* provider = nullptr;
  std::call_once(once, []() { provider = new PlatformPathProvider(); });
  return provider->GetPath(type);
}

PathProvider::PathProvider() = default;

PathProvider::~PathProvider() = default;

}  // namespace paths
}  // namespace fml
