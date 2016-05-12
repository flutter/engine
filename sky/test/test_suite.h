// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SRC_SKY_TEST_FLUTTER_TEST_SUITE_H_
#define SRC_SKY_TEST_FLUTTER_TEST_SUITE_H_

#include "base/macros.h"
#include "base/test/test_suite.h"

namespace flutter {
namespace test {

class TestSuite : public base::TestSuite {
 public:
  TestSuite(int argc, char** argv);

  ~TestSuite() override;

 private:
  void Initialize() override;

  void Shutdown() override;

  DISALLOW_COPY_AND_ASSIGN(TestSuite);
};

}  // namespace test
}  // namespace flutter

#endif  // SRC_SKY_TEST_FLUTTER_TEST_SUITE_H_
