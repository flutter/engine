// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/darwin/ios/framework/Source/FlutterTextInputPlugin.mm"
#include "gtest/gtest.h"

TEST(FlutterTextInputView, SanitizesSelectedTextRange) {
  FlutterTextInputView* view = [[FlutterTextInputView alloc] init];
  [view insertText:@"😠"];
  [view setSelectedTextRange:[FlutterTextRange rangeWithNSRange:NSMakeRange(1, 0)]];
  FlutterTextRange* selectedRange = (FlutterTextRange*)[view selectedTextRange];
  EXPECT_EQ(selectedRange.range.location, 2);
}
