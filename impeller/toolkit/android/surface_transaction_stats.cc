// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/android/surface_transaction_stats.h"

#include "impeller/toolkit/android/proc_table.h"

namespace impeller::android {

fml::UniqueFD CreatePreviousReleaseFence(const SurfaceControl& control,
                                         int release_file_fd) {
  const auto fd = release_file_fd;
  if (fd == -1) {
    // The previous buffer has already been released. This is not an error.
    return {};
  }
  return fml::UniqueFD{fd};
}

}  // namespace impeller::android
