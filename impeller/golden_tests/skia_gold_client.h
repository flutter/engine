// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstdint>
#include <memory>
#include <string>

namespace impeller {
namespace testing {

struct SkiaGoldClientImpl;

class SkiaGoldClient {
 public:
  SkiaGoldClient();

  virtual ~SkiaGoldClient();

  bool Auth();

  void Compare(const std::string& name, const std::string& png_path);

 private:
  std::unique_ptr<SkiaGoldClientImpl> impl_;
};

}  // namespace testing
}  // namespace impeller
