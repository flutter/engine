// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_DARWIN_PATH_PROVIDER_DARWIN_H_
#define FLUTTER_FML_PLATFORM_DARWIN_PATH_PROVIDER_DARWIN_H_

#include "flutter/fml/paths.h"
#include "lib/ftl/macros.h"

namespace fml {
namespace paths {

class PathProviderDarwin : public PathProvider {
 public:
  PathProviderDarwin();

  ~PathProviderDarwin() override;

  std::pair<bool, std::string> GetPath(PathType type) override;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(PathProviderDarwin);
};

}  // namespace paths
}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_DARWIN_PATH_PROVIDER_DARWIN_H_
