// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FILE_IN_NAMESPACE_BUFFER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FILE_IN_NAMESPACE_BUFFER_H_

#include "flutter/fml/mapping.h"

namespace flutter_runner {

class FileInNamespaceBuffer final : public fml::Mapping {
 public:
  FileInNamespaceBuffer(int namespace_fd, const char* path, bool executable);
  ~FileInNamespaceBuffer();

  // |fml::Mapping|
  const uint8_t* GetMapping() const override;

  // |fml::Mapping|
  size_t GetSize() const override;

 private:
  void* address_;
  size_t size_;

  FML_DISALLOW_COPY_AND_ASSIGN(FileInNamespaceBuffer);
};

std::unique_ptr<fml::Mapping> LoadFile(int namespace_fd,
                                       const char* file_path,
                                       bool executable);

std::unique_ptr<fml::FileMapping> MakeFileMapping(const char* path,
                                                  bool executable);

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FILE_IN_NAMESPACE_BUFFER_H_
