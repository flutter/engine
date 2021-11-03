// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//
#include "flutter/shell/platform/common/text_editing_delta.h"

#include "gtest/gtest.h"

namespace flutter {

// TODO(justinmc): Are there any additional tests to import from the iOS
// FlutterTextEditingDelta tests?
TEST(TextEditingDeltaTest, TestTextEditingDeltaConstructor) {
  // Here we are simulating inserting an "o" at the end of "hell".
  std::string oldText = "hell";
  std::string replacementText = "hello";
  flutter::TextRange range = flutter::TextRange(0, 4);

  TextEditingDelta delta = TextEditingDelta(oldText, range, replacementText);

  ASSERT_EQ(delta.oldText(), oldText);
  ASSERT_EQ(delta.deltaText(), "hello");
  ASSERT_EQ(delta.deltaStart(), 0);
  ASSERT_EQ(delta.deltaEnd(), 4);
}

TEST(TextEditingDeltaTest, TestTextEditingDeltaNonTextConstructor) {
  // Here we are simulating inserting an "o" at the end of "hell".
  std::string oldText = "hello";

  TextEditingDelta delta = TextEditingDelta(oldText);

  ASSERT_EQ(delta.oldText(), oldText);
  ASSERT_EQ(delta.deltaText(), "");
  ASSERT_EQ(delta.deltaStart(), -1);
  ASSERT_EQ(delta.deltaEnd(), -1);
}

}  // namespace flutter
