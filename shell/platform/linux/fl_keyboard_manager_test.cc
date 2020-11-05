// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_manager.h"

#include <iostream>
#include "gtest/gtest.h"

#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"
#include "flutter/shell/platform/linux/testing/mock_renderer.h"

static void expect_eq_physical(const FlKeyDatum& data, const FlKeyDatum& target) {
  EXPECT_EQ(data.kind, target.kind);
  EXPECT_EQ(data.key, target.key);
  EXPECT_EQ(data.repeated, target.repeated);
  EXPECT_EQ(data.timestamp, target.timestamp);
  EXPECT_EQ(data.active_locks, target.active_locks);
  EXPECT_EQ(data.logical_data_count, target.logical_data_count);
}

// Test sending a letter "A";
TEST(FlKeyboardManagerTest, SendKeyEvent) {
  g_autoptr(FlKeyboardManager) manager = fl_keyboard_manager_new();
  FlKeyDatum physical_data[kMaxConvertedKeyData];
  FlLogicalKeyDatum logical_data[kMaxConvertedLogicalKeyData];
  size_t result_size;

  char string[] = "A";
  GdkEventKey key_event = GdkEventKey{
      GDK_KEY_PRESS,                         // event type
      nullptr,                               // window (not needed)
      FALSE,                                 // event was sent explicitly
      12345,                                 // time
      0x0,                                   // modifier state
      GDK_KEY_a,                             // key code
      1,                                     // length of string representation
      reinterpret_cast<gchar*>(&string[0]),  // string representation
      0x026,                                 // scan code
      0,                                     // keyboard group
      0,                                     // is a modifier
  };

  result_size = fl_keyboard_manager_convert_key_event(manager, &key_event,
      physical_data, logical_data);

  EXPECT_EQ(result_size, 1u);
  expect_eq_physical(physical_data[0], FlKeyDatum{
    kFlKeyDataKindDown,       // kind
    0x00070004,               // key
    false,                    // repeated
    12345000.0,               // timestamp
    0,                        // active locks
    1,                        // logical count
  });
}
