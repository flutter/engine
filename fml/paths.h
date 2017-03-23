// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PATH_SERVICE_H_
#define FLUTTER_FML_PATH_SERVICE_H_

#include <memory>
#include <string>
#include <utility>

#include "lib/ftl/macros.h"

namespace fml {
namespace paths {

enum class PathType {
  ExecutableDirectory,
};

std::pair<bool, std::string> GetPath(PathType type);

class PathProvider {
 public:
  PathProvider();

  virtual ~PathProvider();

  virtual std::pair<bool, std::string> GetPath(PathType type) = 0;

 private:
  FTL_DISALLOW_COPY_AND_ASSIGN(PathProvider);
};

}  // namespace paths

}  // namespace fml

#endif  // FLUTTER_FML_PATH_SERVICE_H_
