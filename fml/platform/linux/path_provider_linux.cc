// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/linux/path_provider_linux.h"

#include <unistd.h>

#include "lib/ftl/files/path.h"

namespace fml {
namespace paths {

PathProviderLinux::PathProviderLinux() = default;

PathProviderLinux::~PathProviderLinux() = default;

std::pair<bool, std::string> PathProviderLinux::GetPath(PathType type) {
  switch (type) {
    case PathType::ExecutableDirectory: {
      const int pathSize = 255;
      char path[pathSize] = {0};
      auto readSize = ::readlink("/proc/self/exe", path, pathSize);
      if (readSize == -1) {
        return {false, ""};
      }
      return {true, files::GetDirectoryName(std::string{path, readSize})};
    } break;
  }
  return {false, ""};
}

}  // namespace paths
}  // namespace fml
