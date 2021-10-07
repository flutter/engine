// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/common/framework/Headers/FlutterMacros.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterTextEditingDelta.h"
#import "flutter/testing/testing.h"

FLUTTER_ASSERT_ARC

namespace flutter::testing {

TEST(FlutterTextEditingDeltaTest, TestTextEditingDeltaConstructor) {
  // Here we are simulating inserting an "o" at the end of "hell".
  NSString* oldText = @"hell";
  NSString* replacementText = @"hello";
  NSRange range = NSMakeRange(0, 4);

  FlutterTextEditingDelta* delta = [FlutterTextEditingDelta textEditingDelta:oldText
                                                               replacedRange:range
                                                                 updatedText:replacementText];

  ASSERT_EQ(delta.oldText, oldText);
  ASSERT_EQ(delta.deltaText, @"hello");
  ASSERT_EQ(delta.deltaStart, 0);
  ASSERT_EQ(delta.deltaEnd, 4);
}

TEST(FlutterTextEditingDeltaTest, TestTextEditingDeltaNonTextConstructor) {
  // Here we are simulating inserting an "o" at the end of "hell".
  NSString* oldText = @"hello";

  FlutterTextEditingDelta* delta = [FlutterTextEditingDelta deltaWithNonText:oldText];

  ASSERT_EQ(delta.oldText, oldText);
  ASSERT_EQ(delta.deltaText, @"");
  ASSERT_EQ(delta.deltaStart, -1);
  ASSERT_EQ(delta.deltaEnd, -1);
}

}
