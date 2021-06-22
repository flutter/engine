// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fsl/vmo/file.h"

#include <fcntl.h>
#include <lib/fdio/io.h>
#include <lib/syslog/cpp/macros.h>
#include <sys/stat.h>
#include <unistd.h>

namespace fsl {

bool VmoFromFd(fbl::unique_fd fd, SizedVmo* handle_ptr) {
  FX_CHECK(handle_ptr);

  struct stat stat_struct;
  if (fstat(fd.get(), &stat_struct) == -1)
    return false;
  zx_handle_t result = ZX_HANDLE_INVALID;
  zx_status_t status = fdio_get_vmo_copy(fd.get(), &result);
  if (status != ZX_OK)
    return false;
  *handle_ptr = SizedVmo(zx::vmo(result), stat_struct.st_size);
  return true;
}

bool VmoFromFilename(const std::string& filename, SizedVmo* handle_ptr) {
  int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1)
    return false;
  return VmoFromFd(fbl::unique_fd(fd), handle_ptr);
}

bool VmoFromFilenameAt(int dirfd, const std::string& filename, SizedVmo* handle_ptr) {
  int fd = openat(dirfd, filename.c_str(), O_RDONLY);
  if (fd == -1)
    return false;
  return VmoFromFd(fbl::unique_fd(fd), handle_ptr);
}

}  // namespace fsl
