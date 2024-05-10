// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/dl_builder.h"
#include "flutter/shell/common/display_list_debugger.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DisplayListDebuggerTest, Noop) {
  DisplayListBuilder builder;
  sk_sp<DisplayList> display_list = builder.Build();
  EXPECT_FALSE(DisplayListDebugger::SaveDisplayList(display_list).ok());
}

TEST(DisplayListDebuggerTest, Simple) {
  DisplayListBuilder builder;
  builder.Scale(0.2, 0.2);
  sk_sp<DisplayList> display_list = builder.Build();
  fml::ScopedTemporaryDirectory temp_dir;
  auto message = std::make_unique<PlatformMessage>(
      DisplayListDebugger::kChannelName,
      fml::MallocMapping::Copy(temp_dir.path().c_str(),
                               temp_dir.path().size() + 1),
      /*response=*/nullptr);
  DisplayListDebugger::HandleMessage(std::move(message));
  EXPECT_TRUE(DisplayListDebugger::SaveDisplayList(display_list).ok());
}

}  // namespace testing
}  // namespace flutter
