// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/files/directory.h"

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/stat.h>
#include <unistd.h>

#include <string>
#include <vector>

#include "src/lib/files/path.h"

namespace files {

std::string GetCurrentDirectory() {
  char buffer[PATH_MAX];
  if (!getcwd(buffer, sizeof(buffer))) {
    // Failed to get cwd
    abort();
  }
  return std::string(buffer);
}

bool IsDirectory(const std::string& path) { return IsDirectoryAt(AT_FDCWD, path); }

bool IsDirectoryAt(int root_fd, const std::string& path) {
  struct stat buf;
  if (fstatat(root_fd, path.c_str(), &buf, 0) != 0)
    return false;
  return S_ISDIR(buf.st_mode);
}

bool CreateDirectory(const std::string& full_path) {
  return CreateDirectoryAt(AT_FDCWD, full_path);
}

bool CreateDirectoryAt(int root_fd, const std::string& full_path) {
  std::vector<std::string> subpaths;

  // Collect a list of all parent directories.
  std::string last_path = full_path;
  subpaths.push_back(full_path);
  for (std::string path = GetDirectoryName(full_path); !path.empty() && path != last_path;
       path = GetDirectoryName(path)) {
    subpaths.push_back(path);
    last_path = path;
  }

  // Iterate through the parents and create the missing ones.
  for (auto it = subpaths.rbegin(); it != subpaths.rend(); ++it) {
    if (IsDirectoryAt(root_fd, *it))
      continue;
    if (mkdirat(root_fd, it->c_str(), 0700) == 0)
      continue;
    // Mkdir failed, but it might be due to the directory appearing out of thin
    // air. This can occur if two processes are trying to create the same file
    // system tree at the same time. Check to see if it exists and make sure it
    // is a directory.
    if (!IsDirectoryAt(root_fd, *it))
      return false;
  }
  return true;
}

bool ReadDirContents(const std::string& path, std::vector<std::string>* out) {
  return ReadDirContentsAt(AT_FDCWD, path, out);
}

bool ReadDirContentsAt(int root_fd, const std::string& path, std::vector<std::string>* out) {
  out->clear();
  int fd = openat(root_fd, path.c_str(), O_RDONLY | O_DIRECTORY);
  if (fd < 0) {
    return false;
  }
  DIR* dir = fdopendir(fd);
  if (!dir) {
    close(fd);
    return false;
  }
  struct dirent* de;
  errno = 0;
  while ((de = readdir(dir)) != nullptr) {
    out->push_back(de->d_name);
  }
  closedir(dir);
  return !errno;
}

}  // namespace files
