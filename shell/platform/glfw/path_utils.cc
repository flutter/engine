// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/glfw/path_utils.h"

#include <linux/limits.h>
#include <unistd.h>

namespace flutter {

std::filesystem::path GetExecutableDirectory() {
  char buffer[PATH_MAX + 1];
  ssize_t length = readlink("/proc/self/exe", buffer, sizeof(buffer));
  if (length > PATH_MAX) {
    return std::filesystem::path();
  }
  std::filesystem::path executable_path(std::string(buffer, length));
  return executable_path.remove_filename();
}

}  // namespace flutter
