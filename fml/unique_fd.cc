// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/unique_fd.h"

#include "flutter/fml/eintr_wrapper.h"

namespace fml {
namespace internal {

#if OS_WIN

namespace os_win {

std::mutex UniqueFDTraits::file_map_mutex;
std::map<HANDLE, DirCacheEntry> UniqueFDTraits::file_map;

void UniqueFDTraits::Free_Handle(HANDLE fd) {
  CloseHandle(fd);
}

}  // namespace os_win

#endif  // OS_WIN

#if OS_POSIX || OS_FUCHSIA

namespace os_unix {

void UniqueFDTraits::Free(int fd) {
  close(fd);
}

#ifndef FLUTTER_NO_IO
void UniqueDirTraits::Free(DIR* dir) {
  closedir(dir);
}
#endif

}  // namespace os_unix

#endif  // OS_POSIX || OS_FUCHSIA

}  // namespace internal
}  // namespace fml
