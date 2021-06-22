// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/files/file_descriptor.h"

#include <lib/syslog/cpp/macros.h>

#include "src/lib/files/eintr_wrapper.h"

namespace fxl {

bool WriteFileDescriptor(int fd, const char* data, ssize_t size) {
  ssize_t total = 0;
  for (ssize_t partial = 0; total < size; total += partial) {
    partial = HANDLE_EINTR(write(fd, data + total, size - total));
    if (partial < 0)
      return false;
  }
  return true;
}

ssize_t ReadFileDescriptor(int fd, char* data, ssize_t max_size) {
  ssize_t total = 0;
  for (ssize_t partial = 0; total < max_size; total += partial) {
    partial = HANDLE_EINTR(read(fd, data + total, max_size - total));
    if (partial <= 0)
      return total ? total : partial;
  }
  return total;
}

}  // namespace fxl
