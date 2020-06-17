// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/jni/mock_jni.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(MockJNI, FlutterViewHandlePlatformMessage) {
  MockJNI mock_jni;

  auto message =
      fml::MakeRefCounted<PlatformMessage>("<channel-name>", nullptr);
  auto response_id = 1;

  EXPECT_CALL(mock_jni, FlutterViewHandlePlatformMessage(message, response_id));

  mock_jni.FlutterViewHandlePlatformMessage(message, response_id);
}

}  // namespace testing
}  // namespace flutter
