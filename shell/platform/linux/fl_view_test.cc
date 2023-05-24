// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Included first as it collides with the X11 headers.
#include "gtest/gtest.h"

#include "flutter/shell/platform/embedder/test_utils/proc_table_replacement.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/fl_pointer_event.h"
#include "flutter/shell/platform/linux/fl_view_private.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"

// MOCK_ENGINE_PROC is leaky by design
// NOLINTBEGIN(clang-analyzer-core.StackAddressEscape)

TEST(FlEngineTest, SendPointerEventMouse) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  FlutterEngineProcTable* embedder_api = fl_engine_get_embedder_api(engine);

  bool called = false;
  embedder_api->SendPointerEvent = MOCK_ENGINE_PROC(
      SendPointerEvent,
      ([&called](auto engine, const FlutterPointerEvent* events,
                 size_t events_count) {
        called = true;
        EXPECT_EQ(events_count, static_cast<size_t>(1));
        EXPECT_EQ(events[0].phase, kDown);
        EXPECT_EQ(events[0].timestamp, static_cast<size_t>(1234567890));
        EXPECT_EQ(events[0].x, 800);
        EXPECT_EQ(events[0].y, 600);
        EXPECT_EQ(events[0].pressure, 0.0);
        EXPECT_EQ(events[0].device, static_cast<int32_t>(0));
        EXPECT_EQ(events[0].signal_kind, kFlutterPointerSignalKindNone);
        EXPECT_EQ(events[0].scroll_delta_x, 0.0);
        EXPECT_EQ(events[0].scroll_delta_y, 0.0);
        EXPECT_EQ(events[0].device_kind, kFlutterPointerDeviceKindMouse);
        EXPECT_EQ(events[0].buttons, kFlutterPointerButtonMousePrimary);

        return kSuccess;
      }));

  g_autoptr(GError) error = nullptr;
  EXPECT_TRUE(fl_engine_start(engine, &error));
  EXPECT_EQ(error, nullptr);

  FlDartProject* project = fl_dart_project_new();
  FlView* view = fl_view_new_with_engine(project, engine);

  GdkEvent* gdk_event = gdk_event_new(GDK_BUTTON_PRESS);
  gdk_event->button.time = 1234567890;
  gdk_event->button.x = 800;
  gdk_event->button.y = 900;
  gdk_event->button.axes = NULL;
  gdk_event->button.button = 1;

  FlPointerEvent* fl_event =
      fl_pointer_event_new_from_gdk_event(gdk_event, view);
  fl_view_send_pointer_event(view, fl_event);
  EXPECT_TRUE(called);
}

TEST(FlEngineTest, SendPointerEventStylus) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  FlutterEngineProcTable* embedder_api = fl_engine_get_embedder_api(engine);

  bool called = false;
  embedder_api->SendPointerEvent = MOCK_ENGINE_PROC(
      SendPointerEvent,
      ([&called](auto engine, const FlutterPointerEvent* events,
                 size_t events_count) {
        called = true;
        EXPECT_EQ(events_count, static_cast<size_t>(1));
        EXPECT_EQ(events[0].phase, kDown);
        EXPECT_EQ(events[0].timestamp, static_cast<size_t>(1234567890));
        EXPECT_EQ(events[0].x, 800);
        EXPECT_EQ(events[0].y, 600);
        EXPECT_EQ(events[0].pressure, 0.5);
        EXPECT_EQ(events[0].device, static_cast<int32_t>(0));
        EXPECT_EQ(events[0].signal_kind, kFlutterPointerSignalKindNone);
        EXPECT_EQ(events[0].scroll_delta_x, 0.0);
        EXPECT_EQ(events[0].scroll_delta_y, 0.0);
        EXPECT_EQ(events[0].device_kind, kFlutterPointerDeviceKindStylus);
        EXPECT_EQ(events[0].buttons, kFlutterPointerButtonMousePrimary);

        return kSuccess;
      }));

  g_autoptr(GError) error = nullptr;
  EXPECT_TRUE(fl_engine_start(engine, &error));
  EXPECT_EQ(error, nullptr);

  FlDartProject* project = fl_dart_project_new();
  FlView* view = fl_view_new_with_engine(project, engine);

  GdkEvent* gdk_event = gdk_event_new(GDK_BUTTON_PRESS);
  gdk_event->button.time = 1234567890;
  gdk_event->button.x = 800;
  gdk_event->button.y = 900;
  gdk_event->button.axes = NULL;  // TODO: Define pressure
  gdk_event->button.button = 1;

  FlPointerEvent* fl_event =
      fl_pointer_event_new_from_gdk_event(gdk_event, view);
  fl_view_send_pointer_event(view, fl_event);
  EXPECT_TRUE(called);
}

// NOLINTEND(clang-analyzer-core.StackAddressEscape)
