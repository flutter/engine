// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_LINUX_PATH_PROVIDER_LINUX_H_
#define FLUTTER_FML_PLATFORM_LINUX_PATH_PROVIDER_LINUX_H_

#include "flutter/fml/paths.h"
#include "lib/ftl/macros.h"

namespace fml {
namespace paths {

class PathProviderLinux : public PathProvider {
 public:
  PathProviderLinux();

  ~PathProviderLinux() override;

  std::pair<bool, std::string> GetPath(PathType type) override;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(PathProviderLinux);
};

}  // namespace paths
}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_LINUX_PATH_PROVIDER_LINUX_H_
