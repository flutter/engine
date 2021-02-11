// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/fml/synchronization/sync_switch.h"

#include "gtest/gtest.h"

using fml::SyncSwitch;

TEST(SyncSwitchTest, Basic) {
  auto sync_switch = std::make_shared<SyncSwitch>();
  SyncSwitch::Controller controller(sync_switch);
  bool switch_value = false;
  sync_switch->Execute(SyncSwitch::Handlers()
                           .SetIfTrue([&] { switch_value = true; })
                           .SetIfFalse([&] { switch_value = false; }));
  EXPECT_FALSE(switch_value);
  controller.SetSwitch(true);
  sync_switch->Execute(SyncSwitch::Handlers()
                           .SetIfTrue([&] { switch_value = true; })
                           .SetIfFalse([&] { switch_value = false; }));
  EXPECT_TRUE(switch_value);
}

TEST(SyncSwitchTest, NoopIfUndefined) {
  auto sync_switch = std::make_shared<SyncSwitch>();
  SyncSwitch::Controller controller(sync_switch);
  bool switch_value = false;
  sync_switch->Execute(SyncSwitch::Handlers());
  EXPECT_FALSE(switch_value);
}
