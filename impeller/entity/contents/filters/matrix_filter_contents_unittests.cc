// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/testing/testing.h"
#include "gmock/gmock.h"
#include "impeller/entity/contents/filters/matrix_filter_contents.h"

#if FML_OS_MACOSX
#define IMPELLER_RAND arc4random
#else
#define IMPELLER_RAND rand
#endif

namespace impeller {
namespace testing {
TEST(MatrixFilterContentsTest, Create) {
  MatrixFilterContents contents;
  EXPECT_TRUE(contents.IsTranslationOnly());
}

}  // namespace testing
}  // namespace impeller
