// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <string>
#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/impeller/golden_tests/working_directory.h"

namespace impeller {
namespace testing {

class GoldenDigest {
 public:
  static GoldenDigest* Instance();

  void AddImage(const std::string& test_name,
                const std::string& filename,
                int32_t width,
                int32_t height);

  bool Write(WorkingDirectory* working_directory);

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(GoldenDigest);
  GoldenDigest();
  struct Entry {
    std::string test_name;
    std::string filename;
    int32_t width;
    int32_t height;
  };

  static GoldenDigest* instance_;
  std::vector<Entry> entries_;
};
}  // namespace testing
}  // namespace impeller
