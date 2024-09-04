// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/dl_builder.h"

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DlBuilderTest, RestoreWithoutSave) {
  auto builder = std::make_unique<DisplayListBuilder>();
  builder->Restore();
  sk_sp<DisplayList> display_list = builder->Build();
  ASSERT_TRUE(display_list);
}

TEST(DlBuilderTest, EmptySaveRestore) {
  auto builder = std::make_unique<DisplayListBuilder>();
  builder->Save();
  builder->Restore();
  sk_sp<DisplayList> display_list = builder->Build();
  ASSERT_TRUE(display_list);
}

TEST(DlBuilderTest, EmptySaveTranslateRestore) {
  auto builder = std::make_unique<DisplayListBuilder>();
  builder->Save();
  builder->Translate(10, 10);
  builder->Restore();
  sk_sp<DisplayList> display_list = builder->Build();
  ASSERT_TRUE(display_list);
}

}  // namespace testing
}  // namespace flutter