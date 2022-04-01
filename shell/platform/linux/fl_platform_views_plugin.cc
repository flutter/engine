// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_platform_views_plugin.h"

#include <gtk/gtk.h>
#include <cstring>

#include "flutter/shell/platform/linux/fl_platform_view_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_method_codec.h"

static constexpr char kChannelName[] = "flutter/platform_views";
static constexpr char kBadArgumentsError[] = "Bad Arguments";
static constexpr char kCreateMethod[] = "create";
static constexpr char kDisposeMethod[] = "dispose";
static constexpr char kAcceptGestureMethod[] = "acceptGesture";
static constexpr char kRejectGestureMethod[] = "rejectGesture";
static constexpr char kSetDirectionMethod[] = "setDirection";
static constexpr char kEnterMethod[] = "enter";
static constexpr char kExitMethod[] = "exit";

struct _FlPlatformViewsPlugin {
  GObject parent_instance;

  // Method channel for communication with Flutter Framework
  // PlatformViewsService.
  FlMethodChannel* channel;

  // Internal record for the platform view factories.
  //
  // It is a map from platform view type to platform view factory registered by
  // plugins. The keys are strings.  The values are pointer to
  // #FlPlatformViewFactory.  This table is freed by the responder.
  GHashTable* factories;

  // Internal record for the onscreen platform views.
  //
  // It is a map from Flutter platform view ID to #FlPlatformView instance
  // created by plugins.  The keys are directly stored int64s. The values are
  // stored pointer to #FlPlatformView.  This table is freed by the responder.
  GHashTable* platform_views;
};

G_DEFINE_TYPE(FlPlatformViewsPlugin, fl_platform_views_plugin, G_TYPE_OBJECT)

// Sends the method call response to Flutter Framework.
static void send_response(FlMethodCall* method_call,
                          FlMethodResponse* response) {
  g_autoptr(GError) error = nullptr;
  if (!fl_method_call_respond(method_call, response, &error)) {
    g_warning("Failed to send method call response: %s", error->message);
  }
}

// Creates a new platform view.
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

  FlValue* direction_value = fl_value_lookup_string(args, "direction");
  if (direction_value == nullptr ||
      fl_value_get_type(direction_value) != FL_VALUE_TYPE_INT) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Missing platform view direction", nullptr));
  }
  GtkTextDirection direction =
      static_cast<GtkTextDirection>(fl_value_get_int(direction_value));

  FlValue* params_value = fl_value_lookup_string(args, "params");

  FlPlatformView* platform_view =
      fl_platform_views_plugin_get_platform_view(self, id);
  if (platform_view) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Platform view id already exists", nullptr));
  }

  FlPlatformViewFactory* factory =
      FL_PLATFORM_VIEW_FACTORY(g_hash_table_lookup(self->factories, view_type));
  if (!factory) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "View type does not exist", nullptr));
  }

  g_autoptr(FlMessageCodec) codec =
      fl_platform_view_factory_get_create_arguments_codec(factory);
  FlValue* creation_params = nullptr;
  if (codec && params_value) {
    const uint8_t* creation_params_bytes =
        fl_value_get_uint8_list(params_value);
    size_t creation_params_length = fl_value_get_length(params_value);
    g_autoptr(GBytes) bytes =
        g_bytes_new_static(creation_params_bytes, creation_params_length);

    g_autoptr(GError) error = nullptr;
    creation_params = fl_message_codec_decode_message(codec, bytes, &error);
    if (!creation_params) {
      g_warning("Failed to decode creation params, error message: %s",
                error->message);
      return FL_METHOD_RESPONSE(fl_method_error_response_new(
          kBadArgumentsError, "Creation params cannot be parsed", nullptr));
    }
  }

  platform_view = fl_platform_view_factory_create_platform_view(
      factory, id, creation_params);
  if (!platform_view) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Invalid platform view", nullptr));
  }
  fl_platform_view_set_direction(platform_view, direction);

  g_hash_table_insert(self->platform_views, GINT_TO_POINTER(id), platform_view);

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

// Called when Flutter considers this gesture applied to a platform view.
static FlMethodResponse* platform_views_accept_gesture(
    FlPlatformViewsPlugin* self,
    FlValue* args) {
  if (fl_value_get_type(args) != FL_VALUE_TYPE_LIST ||
      fl_value_get_length(args) != 2) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Argument list missing or malformed", nullptr));
  }

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

// Reject gesture captured by the platform view.
static FlMethodResponse* platform_views_reject_gesture(
    FlPlatformViewsPlugin* self,
    FlValue* args) {
  if (fl_value_get_type(args) != FL_VALUE_TYPE_LIST ||
      fl_value_get_length(args) != 1) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Argument list missing or malformed", nullptr));
  }

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

// Set GtkTextDirection of platform view.
static FlMethodResponse* platform_views_set_direction(
    FlPlatformViewsPlugin* self,
    FlValue* args) {
  if (fl_value_get_type(args) != FL_VALUE_TYPE_LIST ||
      fl_value_get_length(args) != 2) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Argument list missing or malformed", nullptr));
  }

  int64_t view_id = fl_value_get_int(fl_value_get_list_value(args, 0));
  GtkTextDirection direction = static_cast<GtkTextDirection>(
      fl_value_get_int(fl_value_get_list_value(args, 1)));

  FlPlatformView* platform_view =
      fl_platform_views_plugin_get_platform_view(self, view_id);
  if (!platform_view) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Platform view not found", nullptr));
  }

  fl_platform_view_set_direction(platform_view, direction);

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

// Mark that a mouse pointer has entered into the platform view.
static FlMethodResponse* platform_views_enter(FlPlatformViewsPlugin* self,
                                              FlValue* args) {
  if (fl_value_get_type(args) != FL_VALUE_TYPE_LIST ||
      fl_value_get_length(args) != 1) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Argument list missing or malformed", nullptr));
  }

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

// Mark that a mouse pointer has exited from the platform view.
static FlMethodResponse* platform_views_exit(FlPlatformViewsPlugin* self,
                                             FlValue* args) {
  if (fl_value_get_type(args) != FL_VALUE_TYPE_LIST ||
      fl_value_get_length(args) != 1) {
    return FL_METHOD_RESPONSE(fl_method_error_response_new(
        kBadArgumentsError, "Argument list missing or malformed", nullptr));
  }

  return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
}

// Disposes the platform view.
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
  } else if (strcmp(method, kSetDirectionMethod) == 0) {
    response = platform_views_set_direction(self, args);
  } else if (strcmp(method, kEnterMethod) == 0) {
    response = platform_views_enter(self, args);
  } else if (strcmp(method, kExitMethod) == 0) {
    response = platform_views_exit(self, args);
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
  g_clear_pointer(&self->factories, g_hash_table_unref);
  g_clear_pointer(&self->platform_views, g_hash_table_unref);

  G_OBJECT_CLASS(fl_platform_views_plugin_parent_class)->dispose(object);
}

static void fl_platform_views_plugin_class_init(
    FlPlatformViewsPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_platform_views_plugin_dispose;
}

static void fl_platform_views_plugin_init(FlPlatformViewsPlugin* self) {
  self->factories =
      g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_object_unref);
  self->platform_views = g_hash_table_new_full(g_direct_hash, g_direct_equal,
                                               nullptr, g_object_unref);
}

FlPlatformViewsPlugin* fl_platform_views_plugin_new(
    FlBinaryMessenger* messenger) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);

  FlPlatformViewsPlugin* self = FL_PLATFORM_VIEWS_PLUGIN(
      g_object_new(fl_platform_views_plugin_get_type(), nullptr));

  g_autoptr(FlStandardMethodCodec) codec = fl_standard_method_codec_new();
  self->channel =
      fl_method_channel_new(messenger, kChannelName, FL_METHOD_CODEC(codec));
  fl_method_channel_set_method_call_handler(self->channel, method_call_cb, self,
                                            nullptr);

  return self;
}

gboolean fl_platform_views_plugin_register_view_factory(
    FlPlatformViewsPlugin* self,
    FlPlatformViewFactory* factory,
    const gchar* view_type) {
  return g_hash_table_insert(self->factories, g_strdup(view_type),
                             g_object_ref(factory));
}

FlPlatformView* fl_platform_views_plugin_get_platform_view(
    FlPlatformViewsPlugin* self,
    int64_t view_identifier) {
  return reinterpret_cast<FlPlatformView*>(g_hash_table_lookup(
      self->platform_views, GINT_TO_POINTER(view_identifier)));
}
