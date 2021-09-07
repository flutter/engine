// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/fixture_test.h"

#include "flutter/testing/fixtures_base.h"

namespace flutter {
namespace testing {

FixtureTest::FixtureTest() : FixturesBase() {}

FixtureTest::FixtureTest(std::string kernel_filename,
                         std::string elf_filename,
                         std::string elf_split_filename)
    : FixturesBase(kernel_filename, elf_filename, elf_split_filename) {}

}  // namespace testing
}  // namespace flutter
