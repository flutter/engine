// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_platform_views_plugin.h"

#include <gtk/gtk.h>
#include <cstring>

#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_method_codec.h"

static constexpr char kChannelName[] = "flutter/platform_views";
static constexpr char kBadArgumentsError[] = "Bad Arguments";
static constexpr char kCreateMethod[] = "create";
static constexpr char kDisposeMethod[] = "dispose";
static constexpr char kAcceptGestureMethod[] = "acceptGesture";
static constexpr char kRejectGestureMethod[] = "rejectGesture";
static constexpr char kKeyMethod[] = "key";

struct _FlPlatformViewsPlugin {
  GObject parent_instance;

  FlMethodChannel* channel;

  GdkSeat* seat;

  FlGestureHelper* gesture_helper;

  GHashTable* factories;  // string -> FlPlatformViewFactory*

  GHashTable* platform_views;  // int -> FlPlatformView*
};

G_DEFINE_TYPE(FlPlatformViewsPlugin, fl_platform_views_plugin, G_TYPE_OBJECT)

// Sends the method call response to Flutter.
static void send_response(FlMethodCall* method_call,
                          FlMethodResponse* response) {
  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error)) {
    g_warning("Failed to send method call response: %s", error->message);
  }
}

static FlPlatformView* get_platform_view(FlPlatformViewsPlugin* self,
                                         int64_t id) {
  gpointer p = g_hash_table_lookup(self->platform_views, GINT_TO_POINTER(id));
  if (p && FL_IS_PLATFORM_VIEW(p)) {
    return FL_PLATFORM_VIEW(p);
  } else {
    return nullptr;
  }
}

// Create a platform view
static FlMethodResponse* platform_views_create(FlPlatformViewsPlugin* self,
                                               FlValue* args) {
  if (fl_value_get_type(args) != FL_VALUE_TYPE_MAP) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Argument map missing or malformed", nullptr));
  }

  FlValue* id_value = fl_value_lookup_string(args, "id");
  if (id_value == nullptr || fl_value_get_type(id_value) != FL_VALUE_TYPE_INT) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Missing platform view id", nullptr));
  }
  int64_t id = fl_value_get_int(id_value);

  FlValue* view_type_value = fl_value_lookup_string(args, "viewType");
  if (view_type_value == nullptr ||
      fl_value_get_type(view_type_value) != FL_VALUE_TYPE_STRING) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Missing platform view viewType", nullptr));
  }
  const gchar* view_type = fl_value_get_string(view_type_value);

  FlValue* params_value = fl_value_lookup_string(args, "params");

  FlPlatformView* platform_view = get_platform_view(self, id);
  if (platform_view) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Platform view id already exists", nullptr));
  }

  gpointer factory_pointer = g_hash_table_lookup(self->factories, view_type);
  if (!factory_pointer) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "View type does not exist", nullptr));
  }
  FlPlatformViewFactory* factory = FL_PLATFORM_VIEW_FACTORY(factory_pointer);

  g_autoptr(FlMessageCodec) codec =
      fl_platform_view_factory_get_create_arguments_codec(factory);
  FlValue* creation_params = nullptr;
  if (codec && params_value) {
    const uint8_t* creation_params_bytes =
        fl_value_get_uint8_list(params_value);
    size_t creation_params_length = fl_value_get_length(params_value);
    g_autoptr(GBytes) bytes =
        g_bytes_new_static(creation_params_bytes, creation_params_length);

    GError* error = nullptr;
    creation_params = fl_message_codec_decode_message(codec, bytes, &error);
  }

  platform_view = fl_platform_view_factory_create_platform_view(
      factory, id, creation_params);
  if (!FL_IS_PLATFORM_VIEW(platform_view)) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Invalid platform view", nullptr));
  }

  g_hash_table_insert(self->platform_views, GINT_TO_POINTER(id), platform_view);

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

enum Action : int {
  kActionDown = 0,
  kActionUp = 1,
  kActionMove = 2,
  kActionCancel = 3,
  kActionPointerDown = 5,
  kActionPointerUp = 6,
};

static FlMethodResponse* platform_views_accept_gesture(
    FlPlatformViewsPlugin* self,
    FlValue* args) {
  if (fl_value_get_type(args) != FL_VALUE_TYPE_LIST) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Argument map missing or malformed", nullptr));
  }

  int64_t view_id = fl_value_get_int(fl_value_get_list_value(args, 0));
  int64_t pointer_id = fl_value_get_int(fl_value_get_list_value(args, 1));
  FlPlatformView* platform_view =
      fl_platform_views_plugin_get_platform_view(self, view_id);
  if (!platform_view) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Platform view not found", nullptr));
  }

  GtkWidget* widget = fl_platform_view_get_view(platform_view);
  fl_gesture_helper_accept_gesture(self->gesture_helper, widget, pointer_id);
  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

static FlMethodResponse* platform_views_reject_gesture(
    FlPlatformViewsPlugin* self,
    FlValue* args) {
  if (fl_value_get_type(args) != FL_VALUE_TYPE_LIST) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Argument map missing or malformed", nullptr));
  }

  int64_t view_id = fl_value_get_int(fl_value_get_list_value(args, 0));
  FlPlatformView* platform_view =
      fl_platform_views_plugin_get_platform_view(self, view_id);
  if (!platform_view) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Platform view not found", nullptr));
  }

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

static FlMethodResponse* platform_views_key(FlPlatformViewsPlugin* self,
                                            FlValue* args) {
  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

static FlMethodResponse* platform_views_dispose(FlPlatformViewsPlugin* self,
                                                FlValue* args) {
  if (fl_value_get_type(args) != FL_VALUE_TYPE_INT) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Argument map missing or malformed", nullptr));
  }

  int64_t id = fl_value_get_int(args);

  if (!g_hash_table_remove(self->platform_views, GINT_TO_POINTER(id))) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Platform view does not exist", nullptr));
  }

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

// Called when a method call is received from Flutter.
static void method_call_cb(FlMethodChannel* channel,
                           FlMethodCall* method_call,
                           gpointer user_data) {
  FlPlatformViewsPlugin* self = FL_PLATFORM_VIEWS_PLUGIN(user_data);

  const gchar* method = fl_method_call_get_name(method_call);
  FlValue* args = fl_method_call_get_args(method_call);

  g_autoptr(FlMethodResponse) response = nullptr;
  if (strcmp(method, kCreateMethod) == 0) {
    response = platform_views_create(self, args);
  } else if (strcmp(method, kDisposeMethod) == 0) {
    response = platform_views_dispose(self, args);
  } else if (strcmp(method, kAcceptGestureMethod) == 0) {
    response = platform_views_accept_gesture(self, args);
  } else if (strcmp(method, kRejectGestureMethod) == 0) {
    response = platform_views_reject_gesture(self, args);
  } else if (strcmp(method, kKeyMethod) == 0) {
    response = platform_views_key(self, args);
  } else {
    response = FL_METHOD_RESPONSE(fl_method_not_implemented_response_new());
  }

  if (response != nullptr) {
    send_response(method_call, response);
  }
}

static void fl_platform_views_plugin_dispose(GObject* object) {
  FlPlatformViewsPlugin* self = FL_PLATFORM_VIEWS_PLUGIN(object);

  g_clear_object(&self->channel);
  g_hash_table_destroy(self->factories);
  g_hash_table_destroy(self->platform_views);
  self->factories = self->platform_views = nullptr;

  G_OBJECT_CLASS(fl_platform_views_plugin_parent_class)->dispose(object);
}

static void fl_platform_views_plugin_class_init(
    FlPlatformViewsPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_platform_views_plugin_dispose;
}

static void fl_platform_views_plugin_init(FlPlatformViewsPlugin* self) {}

FlPlatformViewsPlugin* fl_platform_views_plugin_new(
    FlBinaryMessenger* messenger,
    FlGestureHelper* gesture_helper) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);

  FlPlatformViewsPlugin* self = FL_PLATFORM_VIEWS_PLUGIN(
      g_object_new(fl_platform_views_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  self->channel =
      fl_method_channel_new(messenger, kChannelName, FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(self->channel, method_call_cb, self,
                                            nullptr);

  GdkDisplay* display = gdk_display_get_default();
  self->seat = gdk_display_get_default_seat(display);
  self->gesture_helper = gesture_helper;
  self->factories = g_hash_table_new(g_str_hash, g_str_equal);
  self->platform_views = g_hash_table_new(g_direct_hash, g_direct_equal);

  return self;
}

gboolean fl_platform_views_plugin_register_view_factory(
    FlPlatformViewsPlugin* self,
    FlPlatformViewFactory* factory,
    const gchar* view_type) {
  return g_hash_table_insert(self->factories, g_strdup(view_type), factory);
}

FlPlatformView* fl_platform_views_plugin_get_platform_view(
    FlPlatformViewsPlugin* self,
    int identifier) {
  return (FlPlatformView*)g_hash_table_lookup(self->platform_views,
                                              GINT_TO_POINTER(identifier));
}
