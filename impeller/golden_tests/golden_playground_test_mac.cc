// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/impeller/golden_tests/golden_playground_test.h"

#include "flutter/impeller/golden_tests/golden_digest.h"
#include "flutter/impeller/golden_tests/metal_screenshoter.h"

namespace impeller {

struct GoldenPlaygroundTest::GoldenPlaygroundTestImpl {};

GoldenPlaygroundTest::GoldenPlaygroundTest()
    : pimpl_(new GoldenPlaygroundTest::GoldenPlaygroundTestImpl()) {}

void GoldenPlaygroundTest::SetUp() {
  if (GetBackend() != PlaygroundBackend::kMetal) {
    GTEST_SKIP_("GoldenPlaygroundTest doesn't support this backend type.");
    return;
  }
}

PlaygroundBackend GoldenPlaygroundTest::GetBackend() const {
  return GetParam();
}

bool GoldenPlaygroundTest::OpenPlaygroundHere(const Picture& picture) {
  return false;
}

bool GoldenPlaygroundTest::OpenPlaygroundHere(AiksPlaygroundCallback callback) {
  return false;
}

std::shared_ptr<Texture> GoldenPlaygroundTest::CreateTextureForFixture(
    const char* fixture_name,
    bool enable_mipmapping) const {
  return nullptr;
}

std::shared_ptr<Context> GoldenPlaygroundTest::GetContext() const {
  return nullptr;
}

Point GoldenPlaygroundTest::GetContentScale() const {
  return Point();
}

Scalar GoldenPlaygroundTest::GetSecondsElapsed() const {
  return Scalar();
}

ISize GoldenPlaygroundTest::GetWindowSize() const {
  return ISize();
}

}  // namespace impeller
