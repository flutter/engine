// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/common/cpp/path_utils.h"

#if defined(__linux__)
#include <linux/limits.h>
#include <unistd.h>
#endif

namespace flutter {

std::filesystem::path GetExecutableDirectory() {
#if defined(__linux__)
  char buffer[PATH_MAX + 1];
  ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer));
  if (length > PATH_MAX) {
    return std::filesystem::path();
  }
  std::filesystem::path executable_path(std::string(buffer, length));
  return executable_path.remove_filename();
#else
  return std::filesystem::path();
#endif
}

}  // namespace flutter
