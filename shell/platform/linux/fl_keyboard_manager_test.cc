// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_event_plugin.h"

#include <iostream>
#include "gtest/gtest.h"

#include "flutter/shell/platform/linux/fl_binary_messenger_private.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_basic_message_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_message_codec.h"
#include "flutter/shell/platform/linux/testing/mock_renderer.h"

// Creates a mock engine that responds to platform messages.
static FlEngine* make_mock_engine() {
  g_autoptr(FlDartProject) project = fl_dart_project_new();
  g_autoptr(FlMockRenderer) renderer = fl_mock_renderer_new();
  g_autoptr(FlEngine) engine = fl_engine_new(project, FL_RENDERER(renderer));
  g_autoptr(GError) engine_error = nullptr;
  EXPECT_TRUE(fl_engine_start(engine, &engine_error));
  EXPECT_EQ(engine_error, nullptr);

  return static_cast<FlEngine*>(g_object_ref(engine));
}

static const char* expected_value = nullptr;

// Test sending a letter "A";
TEST(FlKeyboardManagerTest, SendKeyEvent) {
  make_mock_engine();
  expected_value = nullptr;
}
