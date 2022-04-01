// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_platform_views_plugin.h"

#include "gtest/gtest.h"

#include "flutter/shell/platform/linux/fl_binary_messenger_private.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_message_codec.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_method_codec.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"
#include "flutter/shell/platform/linux/testing/mock_platform_view.h"

static void invoke_response_cb(GObject* source_object,
                               GAsyncResult* res,
                               gpointer user_data) {
  g_main_loop_quit(reinterpret_cast<GMainLoop*>(user_data));
}

// Called when a the test engine notifies us that a response a sent.
static void response_cb(FlBinaryMessenger* messenger,
                        const gchar* channel,
                        GBytes* message,
                        FlBinaryMessengerResponseHandle* response_handle,
                        gpointer user_data) {
  fl_binary_messenger_send_response(messenger, response_handle, nullptr,
                                    nullptr);

  g_main_loop_quit(static_cast<GMainLoop*>(user_data));
}

static void invoke_method(GMainLoop* loop,
                          FlBinaryMessenger* messenger,
                          const gchar* channel_name,
                          const gchar* method_name,
                          FlValue* args,
                          GAsyncReadyCallback cb) {
  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  g_autoptr(FlMethodChannel) channel = fl_method_channel_new(
      messenger, "test/standard-method", FL_METHOD_CODEC(codec));
  g_autoptr(FlValue) invoke_args = fl_value_new_list();
  fl_value_append_take(invoke_args, fl_value_new_string(channel_name));
  fl_value_append_take(invoke_args, fl_value_new_string(method_name));
  fl_value_append_take(invoke_args, args);
  fl_method_channel_invoke_method(channel, "InvokeMethod", invoke_args, nullptr,
                                  cb, loop);
}

static void create_platform_view(GMainLoop* loop,
                                 FlBinaryMessenger* messenger,
                                 int64_t view_id,
                                 const gchar* view_type,
                                 FlValue* args,
                                 GAsyncReadyCallback cb) {
  FlValue* create_args = fl_value_new_map();
  fl_value_set_string_take(create_args, "id", fl_value_new_int(view_id));
  fl_value_set_string_take(create_args, "viewType",
                           fl_value_new_string(view_type));
  fl_value_set_string_take(create_args, "direction",
                           fl_value_new_int(GTK_TEXT_DIR_LTR));
  if (args)
    fl_value_set_string_take(create_args, "params", args);
  invoke_method(loop, messenger, "flutter/platform_views", "create",
                create_args, cb);
}

static void dispose_platform_view(GMainLoop* loop,
                                  FlBinaryMessenger* messenger,
                                  int64_t view_id,
                                  GAsyncReadyCallback cb) {
  invoke_method(loop, messenger, "flutter/platform_views", "dispose",
                fl_value_new_int(view_id), cb);
}

GtkWidget* get_view(FlPlatformView* self) {
  return nullptr;
}

static FlPlatformView* expected_platform_view;
static constexpr int64_t expected_view_identifier = 1;

TEST(FlPlatformViewsPluginTest, CreatePlatformViewWithUnknownViewType) {
  g_autoptr(GMainLoop) loop = g_main_loop_new(nullptr, 0);

  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlBinaryMessenger) messenger = fl_binary_messenger_new(engine);
  g_autoptr(FlPlatformViewsPlugin) plugin =
      fl_platform_views_plugin_new(messenger);

  create_platform_view(loop, messenger, 1, "unknown_view_type", nullptr,
                       invoke_response_cb);

  // Blocks here until invoke_response_cb is called.
  g_main_loop_run(loop);

  EXPECT_EQ(nullptr, fl_platform_views_plugin_get_platform_view(
                         plugin, expected_view_identifier));
}

static FlPlatformView* create_platform_view_with_no_codec(
    FlPlatformViewFactory* factory,
    int64_t view_identifier,
    FlValue* args) {
  EXPECT_EQ(view_identifier, expected_view_identifier);
  EXPECT_EQ(args, nullptr);

  return expected_platform_view =
             FL_PLATFORM_VIEW(fl_mock_platform_view_new(get_view));
}

TEST(FlPlatformViewsPluginTest, CreatePlatformViewWithNoCodec) {
  g_autoptr(GMainLoop) loop = g_main_loop_new(nullptr, 0);

  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlBinaryMessenger) messenger = fl_binary_messenger_new(engine);

  fl_binary_messenger_set_message_handler_on_channel(
      messenger, "test/responses", response_cb, loop, nullptr);

  g_autoptr(FlPlatformViewsPlugin) plugin =
      fl_platform_views_plugin_new(messenger);
  g_autoptr(FlMockViewFactory) factory =
      fl_mock_view_factory_new(create_platform_view_with_no_codec, nullptr);
  fl_platform_views_plugin_register_view_factory(
      plugin, FL_PLATFORM_VIEW_FACTORY(factory), "some_view_type");

  expected_platform_view = nullptr;
  create_platform_view(loop, messenger, expected_view_identifier,
                       "some_view_type", nullptr, nullptr);

  // Blocks here until flutter/platform_views::create is called.
  g_main_loop_run(loop);

  EXPECT_EQ(expected_platform_view, fl_platform_views_plugin_get_platform_view(
                                        plugin, expected_view_identifier));

  dispose_platform_view(loop, messenger, expected_view_identifier,
                        invoke_response_cb);

  // Blocks here until flutter/platform_views::dispose is called.
  g_main_loop_run(loop);
}

static FlPlatformView* create_platform_view_with_codec(
    FlPlatformViewFactory* factory,
    int64_t view_identifier,
    FlValue* args) {
  EXPECT_EQ(view_identifier, expected_view_identifier);
  EXPECT_EQ(fl_value_get_type(args), FL_VALUE_TYPE_STRING);
  EXPECT_STREQ(fl_value_get_string(args), "Hello World!");

  return expected_platform_view =
             FL_PLATFORM_VIEW(fl_mock_platform_view_new(get_view));
}

static FlMessageCodec* get_create_arguments_codec(
    FlPlatformViewFactory* factory) {
  return FL_MESSAGE_CODEC(fl_standard_message_codec_new());
}

TEST(FlPlatformViewsPluginTest, CreatePlatformViewWithCodec) {
  g_autoptr(GMainLoop) loop = g_main_loop_new(nullptr, 0);

  g_autoptr(GError) error = nullptr;
  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlBinaryMessenger) messenger = fl_binary_messenger_new(engine);

  fl_binary_messenger_set_message_handler_on_channel(
      messenger, "test/responses", response_cb, loop, nullptr);

  g_autoptr(FlPlatformViewsPlugin) plugin =
      fl_platform_views_plugin_new(messenger);
  g_autoptr(FlMockViewFactory) factory = fl_mock_view_factory_new(
      create_platform_view_with_codec, get_create_arguments_codec);
  g_autoptr(FlMessageCodec) codec =
      FL_MESSAGE_CODEC(fl_standard_message_codec_new());
  fl_platform_views_plugin_register_view_factory(
      plugin, FL_PLATFORM_VIEW_FACTORY(factory), "some_view_type");

  expected_platform_view = nullptr;
  g_autoptr(FlValue) args = fl_value_new_string("Hello World!");
  g_autoptr(GBytes) bytes =
      fl_message_codec_encode_message(codec, args, &error);
  EXPECT_EQ(error, nullptr);
  create_platform_view(loop, messenger, expected_view_identifier,
                       "some_view_type",
                       fl_value_new_uint8_list_from_bytes(bytes), nullptr);

  // Blocks here until invoke_method_response is called.
  g_main_loop_run(loop);

  EXPECT_EQ(expected_platform_view, fl_platform_views_plugin_get_platform_view(
                                        plugin, expected_view_identifier));

  dispose_platform_view(loop, messenger, expected_view_identifier,
                        invoke_response_cb);

  // Blocks here until flutter/platform_views::dispose is called.
  g_main_loop_run(loop);
}
