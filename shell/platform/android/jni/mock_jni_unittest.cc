// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/android/jni/mock_jni.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(MockJNI, FlutterViewHandlePlatformMessage) {
  auto mock_jni = std::make_unique<MockJNI>();

  auto message =
      fml::MakeRefCounted<PlatformMessage>("<channel-name>", nullptr);
  auto response_id = 1;
  mock_jni->FlutterViewHandlePlatformMessage(message, response_id);

  auto calls = mock_jni->GetCalls();
  ASSERT_EQ(1UL, calls.size());

  auto platform_message_call =
      std::get<MockJNI::FlutterViewHandlePlatformMessageCall>(calls[0]);
  ASSERT_EQ("<channel-name>", platform_message_call.message->channel());
}

}  // namespace testing
}  // namespace flutter
