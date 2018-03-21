// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_FILE_H_
#define FLUTTER_FML_FILE_H_

#include "lib/fxl/files/unique_fd.h"
#include "lib/fxl/macros.h"

namespace fml {

enum class OpenPermission {
  kRead = 1,
  kWrite = 1 << 1,
  kReadWrite = kRead | kWrite,
};

fxl::UniqueFD OpenFile(const char* path,
                       OpenPermission permission,
                       bool is_directory = false);

fxl::UniqueFD OpenFile(const fxl::UniqueFD& base_directory,
                       const char* path,
                       OpenPermission permission,
                       bool is_directory = false);

fxl::UniqueFD Duplicate(int descriptor);

bool IsDirectory(const fxl::UniqueFD& directory);

}  // namespace fml

#endif  // FLUTTER_FML_FILE_H_
