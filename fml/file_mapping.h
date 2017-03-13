// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_FILE_MAPPING_H_
#define FLUTTER_FML_FILE_MAPPING_H_

#include <string>
#include "lib/ftl/macros.h"

namespace fml {

class FileMapping {
 public:
  FileMapping(const std::string& path);

  ~FileMapping();

  size_t GetSize() const;

  const uint8_t* GetMapping() const;

 private:
  size_t size_;
  uint8_t* mapping_;

  FTL_DISALLOW_COPY_AND_ASSIGN(FileMapping);
};

}  // namespace fml

#endif  // FLUTTER_FML_FILE_MAPPING_H_
