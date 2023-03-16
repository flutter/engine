// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/golden_tests/skia_gold_client.h"

#include <cassert>
#include <cstdlib>
#include <filesystem>

namespace flutter {
namespace testing {
const char* GetSourcePath();
const char* GetFixturesPath();
const char* GetTestingAssetsPath();
}  // namespace testing
}  // namespace flutter

namespace impeller {
namespace testing {
struct SkiaGoldClientImpl {
  std::string gold_ctl;
  bool is_luci = false;
  bool did_auth = false;
};

SkiaGoldClient::SkiaGoldClient() : impl_(new SkiaGoldClientImpl()) {
  (void)flutter::testing::GetSourcePath();
}

SkiaGoldClient::~SkiaGoldClient() {}

bool SkiaGoldClient::Auth() {
  impl_->did_auth = true;
  return true;
}

void SkiaGoldClient::Compare(const std::string& name,
                             const std::string& png_path) {
  assert(impl_->did_auth);
}

}  // namespace testing
}  // namespace impeller
