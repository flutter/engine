// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/file_mapping.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include "lib/ftl/files/eintr_wrapper.h"

namespace fml {

FileMapping::FileMapping(const std::string& path)
    : size_(0), mapping_(nullptr) {
  if (path.length() == 0) {
    return;
  }

  int handle = HANDLE_EINTR(::open(path.c_str(), O_RDONLY));

  if (handle == -1) {
    return;
  }

  struct stat stat_buffer = {};

  if (::fstat(handle, &stat_buffer) == 0) {
    if (stat_buffer.st_size > 0) {
      auto mapping = ::mmap(nullptr, stat_buffer.st_size, PROT_READ,
                            MAP_PRIVATE, handle, 0);
      if (mapping != MAP_FAILED) {
        mapping_ = static_cast<uint8_t*>(mapping);
        size_ = stat_buffer.st_size;
      }
    }
  }

  IGNORE_EINTR(::close(handle));
}

FileMapping::~FileMapping() {
  if (mapping_ != nullptr) {
    ::munmap(mapping_, size_);
  }
}

size_t FileMapping::GetSize() const {
  return size_;
}

const uint8_t* FileMapping::GetMapping() const {
  return mapping_;
}

}  // namespace fml
