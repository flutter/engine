// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/platform/darwin/path_provider_darwin.h"

#include <Foundation/Foundation.h>

#include "lib/ftl/files/path.h"

namespace fml {
namespace paths {

PathProviderDarwin::PathProviderDarwin() = default;

PathProviderDarwin::~PathProviderDarwin() = default;

std::pair<bool, std::string> PathProviderDarwin::GetPath(PathType type) {
  switch (type) {
    case PathType::ExecutableDirectory:
      return {true, files::GetDirectoryName(
                        [NSBundle mainBundle].executablePath.UTF8String)};
  }
  return {false, ""};
}

}  // namespace paths
}  // namespace fml
