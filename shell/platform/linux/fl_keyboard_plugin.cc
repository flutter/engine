// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_plugin.h"

#include <gtk/gtk.h>
#include <cstring>

#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_method_codec.h"

static constexpr char kChannelName[] = "flutter/keyboard";
static constexpr char kGetKeyboardStateMethod[] = "getKeyboardState";

struct _FlKeyboardPlugin {
  GObject parent_instance;

  FlMethodChannel* channel;

  FlKeyboardPluginViewDelegate* view_delegate;
};

G_DEFINE_TYPE(FlKeyboardPlugin, fl_keyboard_plugin, G_TYPE_OBJECT)

// Returns the keyboard pressed state.
FlMethodResponse* get_keyboard_state(FlKeyboardPlugin* self) {
  g_autoptr(FlValue) result = fl_value_new_map();

  GHashTable* pressing_records =
      fl_keyboard_plugin_view_delegate_get_keyboard_state(self->view_delegate);

  g_hash_table_foreach(
      pressing_records,
      [](gpointer key, gpointer value, gpointer user_data) {
        int64_t physical_key = reinterpret_cast<int64_t>(key);
        int64_t logical_key = reinterpret_cast<int64_t>(value);
        FlValue* fl_value_map = reinterpret_cast<FlValue*>(user_data);

        fl_value_set_take(fl_value_map, fl_value_new_int(physical_key),
                          fl_value_new_int(logical_key));
      },
      result);
  return FL_METHOD_RESPONSE(fl_method_success_response_new(result));
}

// Called when a method call is received from Flutter.
static void method_call_cb(FlMethodChannel* channel,
                           FlMethodCall* method_call,
                           gpointer user_data) {
  FlKeyboardPlugin* self = FL_KEYBOARD_PLUGIN(user_data);

  const gchar* method = fl_method_call_get_name(method_call);

  g_autoptr(FlMethodResponse) response = nullptr;
  if (strcmp(method, kGetKeyboardStateMethod) == 0) {
    response = get_keyboard_state(self);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error)) {
    g_warning("Failed to send method call response: %s", error->message);
  }
}

static void fl_keyboard_plugin_dispose(GObject* object);

static void fl_keyboard_plugin_class_init(FlKeyboardPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_plugin_dispose;
}

static void fl_keyboard_plugin_init(FlKeyboardPlugin* self) {}

FlKeyboardPlugin* fl_keyboard_plugin_new(
    FlBinaryMessenger* messenger,
    FlKeyboardPluginViewDelegate* view_delegate) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);
  g_return_val_if_fail(FL_IS_KEYBOARD_PLUGIN_VIEW_DELEGATE(view_delegate),
                       nullptr);

  FlKeyboardPlugin* self =
      FL_KEYBOARD_PLUGIN(g_object_new(fl_keyboard_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  self->channel =
      fl_method_channel_new(messenger, kChannelName, FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(self->channel, method_call_cb, self,
                                            nullptr);
  self->view_delegate = view_delegate;
  g_object_add_weak_pointer(
      G_OBJECT(view_delegate),
      reinterpret_cast<gpointer*>(&(self->view_delegate)));

  return self;
}

static void fl_keyboard_plugin_dispose(GObject* object) {
  FlKeyboardPlugin* self = FL_KEYBOARD_PLUGIN(object);

  g_clear_object(&self->channel);
  if (self->view_delegate != nullptr) {
    g_object_remove_weak_pointer(
        G_OBJECT(self->view_delegate),
        reinterpret_cast<gpointer*>(&(self->view_delegate)));
    self->view_delegate = nullptr;
  }

  G_OBJECT_CLASS(fl_keyboard_plugin_parent_class)->dispose(object);
}
