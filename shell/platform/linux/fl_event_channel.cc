// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_event_channel.h"

#include <gmodule.h>

#include "flutter/shell/platform/linux/fl_method_codec_private.h"

static constexpr char kListenMethod[] = "listen";
static constexpr char kCancelMethod[] = "cancel";
static constexpr char kEventRequestError[] = "error";

struct _FlEventChannel {
  GObject parent_instance;

  // Messenger to communicate on.
  FlBinaryMessenger* messenger;

  // Channel name.
  gchar* name;

  // Codec to en/decode messages.
  FlMethodCodec* codec;

  // Function called when the stream is listened to / cancelled.
  FlEventChannelHandler listen_handler;
  FlEventChannelHandler cancel_handler;
  gpointer handler_data;
  GDestroyNotify handler_data_destroy_notify;
};

struct _FlEventChannelResponseHandle {
  GObject parent_instance;

  FlBinaryMessengerResponseHandle* response_handle;
};

// Added here to stop the compiler from optimising this function away.
G_MODULE_EXPORT GType fl_event_channel_get_type();

G_DEFINE_TYPE(FlEventChannel, fl_event_channel, G_TYPE_OBJECT)

// Handle method calls from the Dart side of the channel.
static FlMethodResponse* handle_method_call(FlEventChannel* self,
                                            const gchar* name,
                                            FlValue* args) {
  FlEventChannelHandler handler;
  if (g_strcmp0(name, kListenMethod) == 0) {
    handler = self->listen_handler;
  } else if (g_strcmp0(name, kCancelMethod) == 0) {
    handler = self->cancel_handler;
  } else {
    g_autofree gchar* message =
        g_strdup_printf("Unknown event channel request '%s'", name);
    return FL_METHOD_RESPONSE(
        fl_method_error_response_new(kEventRequestError, message, nullptr));
  }

  // If not handled, just accept requests.
  if (handler == nullptr) {
    return FL_METHOD_RESPONSE(fl_method_success_response_new(nullptr));
  }

  FlMethodResponse* response = handler(self, args, self->handler_data);
  if (response == nullptr) {
    g_autofree gchar* message =
        g_strdup_printf("Event channel request '%s' not responded to", name);
    return FL_METHOD_RESPONSE(
        fl_method_error_response_new(kEventRequestError, message, nullptr));
  }

  // Validate the method response is one of the acceptable types.
  if (FL_IS_METHOD_SUCCESS_RESPONSE(response)) {
    if (fl_method_success_response_get_result(
            FL_METHOD_SUCCESS_RESPONSE(response)) != nullptr) {
      g_warning("Event channel %s responded to '%s' with value", self->name,
                name);
    }
  } else if (FL_IS_METHOD_ERROR_RESPONSE(response)) {
    // All errors are valid.
  } else {
    g_warning("Event channel %s responded to '%s' with response other than ",
              self->name, name);
  }

  return response;
}

// Called when a binary message is received on this channel.
static void message_cb(FlBinaryMessenger* messenger,
                       const gchar* channel,
                       GBytes* message,
                       FlBinaryMessengerResponseHandle* response_handle,
                       gpointer user_data) {
  FlEventChannel* self = FL_EVENT_CHANNEL(user_data);

  g_autofree gchar* name = nullptr;
  g_autoptr(GError) error = nullptr;
  g_autoptr(FlValue) args = nullptr;
  if (!fl_method_codec_decode_method_call(self->codec, message, &name, &args,
                                          &error)) {
    g_warning("Failed to decode message on event channel %s: %s", self->name,
              error->message);
    fl_binary_messenger_send_response(messenger, response_handle, nullptr,
                                      nullptr);
    return;
  }

  g_autoptr(FlMethodResponse) response = handle_method_call(self, name, args);

  g_autoptr(GBytes) data = nullptr;
  if (FL_IS_METHOD_SUCCESS_RESPONSE(response)) {
    FlMethodSuccessResponse* r = FL_METHOD_SUCCESS_RESPONSE(response);
    g_autoptr(GError) codec_error = nullptr;
    data = fl_method_codec_encode_success_envelope(
        self->codec, fl_method_success_response_get_result(r), &codec_error);
    if (data == nullptr) {
      g_warning("Failed to encode event channel %s success response: %s",
                self->name, codec_error->message);
    }
  } else if (FL_IS_METHOD_ERROR_RESPONSE(response)) {
    FlMethodErrorResponse* r = FL_METHOD_ERROR_RESPONSE(response);
    g_autoptr(GError) codec_error = nullptr;
    data = fl_method_codec_encode_error_envelope(
        self->codec, fl_method_error_response_get_code(r),
        fl_method_error_response_get_message(r),
        fl_method_error_response_get_details(r), &codec_error);
    if (data == nullptr) {
      g_warning("Failed to encode event channel %s error response: %s",
                self->name, codec_error->message);
    }
  } else if (FL_IS_METHOD_NOT_IMPLEMENTED_RESPONSE(response)) {
    data = nullptr;
  } else {
    g_assert_not_reached();
  }

  if (!fl_binary_messenger_send_response(messenger, response_handle, data,
                                         &error)) {
    g_warning("Failed to send event channel response: %s", error->message);
  }
}

// Called when the channel handler is closed.
static void channel_closed_cb(gpointer user_data) {
  g_autoptr(FlEventChannel) self = FL_EVENT_CHANNEL(user_data);

  // Disconnect handlers.
  if (self->handler_data_destroy_notify != nullptr) {
    self->handler_data_destroy_notify(self->handler_data);
  }
  self->listen_handler = nullptr;
  self->cancel_handler = nullptr;
  self->handler_data = nullptr;
  self->handler_data_destroy_notify = nullptr;
}

static void fl_event_channel_dispose(GObject* object) {
  FlEventChannel* self = FL_EVENT_CHANNEL(object);

  if (self->messenger != nullptr) {
    fl_binary_messenger_set_message_handler_on_channel(
        self->messenger, self->name, nullptr, nullptr, nullptr);
  }

  g_clear_object(&self->messenger);
  g_clear_pointer(&self->name, g_free);
  g_clear_object(&self->codec);

  if (self->handler_data_destroy_notify != nullptr) {
    self->handler_data_destroy_notify(self->handler_data);
  }
  self->listen_handler = nullptr;
  self->cancel_handler = nullptr;
  self->handler_data = nullptr;
  self->handler_data_destroy_notify = nullptr;

  G_OBJECT_CLASS(fl_event_channel_parent_class)->dispose(object);
}

static void fl_event_channel_class_init(FlEventChannelClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_event_channel_dispose;
}

static void fl_event_channel_init(FlEventChannel* self) {}

G_MODULE_EXPORT FlEventChannel* fl_event_channel_new(
    FlBinaryMessenger* messenger,
    const gchar* name,
    FlMethodCodec* codec,
    FlEventChannelHandler listen_handler,
    FlEventChannelHandler cancel_handler,
    gpointer user_data,
    GDestroyNotify destroy_notify) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);
  g_return_val_if_fail(name != nullptr, nullptr);
  g_return_val_if_fail(FL_IS_METHOD_CODEC(codec), nullptr);

  FlEventChannel* self =
      FL_EVENT_CHANNEL(g_object_new(fl_event_channel_get_type(), nullptr));

  self->messenger = FL_BINARY_MESSENGER(g_object_ref(messenger));
  self->name = g_strdup(name);
  self->codec = FL_METHOD_CODEC(g_object_ref(codec));
  self->listen_handler = listen_handler;
  self->cancel_handler = cancel_handler;
  self->handler_data = user_data;
  self->handler_data_destroy_notify = destroy_notify;

  fl_binary_messenger_set_message_handler_on_channel(
      self->messenger, self->name, message_cb, g_object_ref(self),
      channel_closed_cb);

  return self;
}

G_MODULE_EXPORT gboolean fl_event_channel_send(FlEventChannel* self,
                                               FlValue* event,
                                               GCancellable* cancellable,
                                               GError** error) {
  g_return_val_if_fail(FL_IS_EVENT_CHANNEL(self), FALSE);
  g_return_val_if_fail(event != nullptr, FALSE);

  g_autoptr(GBytes) data =
      fl_method_codec_encode_success_envelope(self->codec, event, error);
  if (data == nullptr) {
    return FALSE;
  }

  fl_binary_messenger_send_on_channel(self->messenger, self->name, data,
                                      cancellable, nullptr, nullptr);

  return TRUE;
}
