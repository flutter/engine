// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_FXL_FILES_DIRECTORY_H_
#define LIB_FXL_FILES_DIRECTORY_H_

#include <string>
#include <vector>

namespace files {

// Returns the current directory. If the current directory cannot be determined,
// this function will terminate the process.
std::string GetCurrentDirectory();

// Returns whether the given path is a directory.
bool IsDirectory(const std::string& path);

// Returns whether the given path is a directory. If |path| is relative, resolve
// it with |root_fd| as reference. See |openat(2)|.
bool IsDirectoryAt(int root_fd, const std::string& path);

// Create a directory at the given path. If necessary, creates any intermediary
// directory.
bool CreateDirectory(const std::string& path);

// Create a directory at the given path. If necessary, creates any intermediary
// directory. If |path| is relative, resolve it with |root_fd| as reference. See
// |openat(2)|.
bool CreateDirectoryAt(int root_fd, const std::string& path);

// List the contents of a directory. If returns false, errno will be set.
bool ReadDirContents(const std::string& path, std::vector<std::string>* out);

// List the contents of a directory. If returns false, errno will be set. If |path| is relative,
// resolve it with |root_fd| as reference. See |openat(2)|.
bool ReadDirContentsAt(int root_fd, const std::string& path, std::vector<std::string>* out);

}  // namespace files

#endif  // LIB_FXL_FILES_DIRECTORY_H_
