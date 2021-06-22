// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/files/glob.h"

namespace files {

Glob::Glob(const std::string& path, const Options& options) {
  memset(&glob_buf_, 0, sizeof(glob_t));
  int flags = options_to_flags(options);
  GlobInternal(path, &flags);
}

Glob::Glob(std::initializer_list<std::string> paths, const Options& options) {
  memset(&glob_buf_, 0, sizeof(glob_t));
  int flags = options_to_flags(options);
  for (const auto& path : paths) {
    GlobInternal(path, &flags);
  }
}

Glob::~Glob() { globfree(&glob_buf_); }

int Glob::options_to_flags(const Options& options) {
  int ret = 0;
  if (options.no_sort) {
    ret |= GLOB_NOSORT;
  }
  if (options.mark) {
    ret |= GLOB_MARK;
  }
  return ret;
}

void Glob::GlobInternal(const std::string& path, int* flags) {
  if (glob(path.c_str(), *flags, nullptr, &glob_buf_) == 0) {
    *flags |= GLOB_APPEND;
  }
}

}  // namespace files
