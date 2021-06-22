// Copyright 2018 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LIB_FXL_FILES_GLOB_H_
#define LIB_FXL_FILES_GLOB_H_

#include <glob.h>
#include <initializer_list>
#include <iterator>
#include <string>

namespace files {

// |Glob| is a C++ wrapper around the library function glob(3). It provides an
// iterator over the globbed file paths.
class Glob {
 public:
  // Options for globbing.
  struct Options {
    // Do not sort resulting paths.
    // GLOB_NOSORT.
    bool no_sort;
    // Append a slash to matched directories.
    // GLOB_MARK.
    bool mark;
  };

  // Iterator over globbed files.
  class iterator : public std::iterator<std::input_iterator_tag, const char*, ptrdiff_t> {
   public:
    iterator(const Glob* glob, size_t offset = 0) : glob_(glob), offset_(offset) {}

    operator bool() const { return offset_ < glob_->glob_buf_.gl_pathc; }
    bool operator==(const iterator& other) const {
      return glob_ == other.glob_ && offset_ == other.offset_;
    }

    void operator++() { ++offset_; }
    void operator--() { --offset_; }
    const char* operator*() {
      if (!(*this)) {
        return nullptr;
      }
      return glob_->glob_buf_.gl_pathv[offset_];
    }

   private:
    const Glob* glob_;
    size_t offset_;
  };

  // Construct a new glob for a given path.
  Glob(const std::string& path, const Options& options = {});
  // Construct a new glob over multiple paths at once.
  Glob(std::initializer_list<std::string> paths, const Options& options = {});
  ~Glob();

  iterator begin() { return iterator(this, 0); }
  iterator end() { return iterator(this, glob_buf_.gl_pathc); }
  size_t size() { return glob_buf_.gl_pathc; }

 private:
  // Convert the options to flags for glob(3).
  static int options_to_flags(const Options& options);
  void GlobInternal(const std::string& path, int* flags);

  glob_t glob_buf_;
};

}  // namespace files

#endif  // LIB_FXL_FILES_GLOB_H_
