// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_LIB_FSL_VMO_FILE_H_
#define SRC_LIB_FSL_VMO_FILE_H_

#include <lib/zx/vmo.h>

#include <string>

#include <fbl/unique_fd.h>

#include "src/lib/fsl/vmo/sized_vmo.h"
#include "src/lib/fxl/fxl_export.h"

namespace fsl {

// Make a new shared buffer with the contents of a file.
FXL_EXPORT bool VmoFromFd(fbl::unique_fd fd, SizedVmo* handle_ptr);

// Make a new shared buffer with the contents of a file.
FXL_EXPORT bool VmoFromFilename(const std::string& filename, SizedVmo* handle_ptr);

// Make a new shared buffer with the contents of a file.
FXL_EXPORT bool VmoFromFilenameAt(int dirfd, const std::string& filename, SizedVmo* handle_ptr);

}  // namespace fsl

#endif  // SRC_LIB_FSL_VMO_FILE_H_
