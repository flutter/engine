// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_channel.h"

#include "flutter/shell/platform/linux/fl_method_codec_private.h"

#include <gmodule.h>

struct _FlMethodChannel {
  GObject parent_instance;

  // Messenger to communicate on
  FlBinaryMessenger* messenger;

  // Channel name
  gchar* name;

  // Codec to en/decode messages
  FlMethodCodec* codec;

  // Function called when a method call is received
  FlMethodChannelMethodCallHandler method_call_handler;
  gpointer method_call_handler_data;
};

// Added here to stop the compiler from optimising this function away
G_MODULE_EXPORT GType fl_method_channel_get_type();

G_DEFINE_TYPE(FlMethodChannel, fl_method_channel, G_TYPE_OBJECT)

// Wrap the binary messenger handle for type safety and to make the API
// consistent
struct _FlMethodChannelResponseHandle {
  FlBinaryMessengerResponseHandle* response_handle;
};

static FlMethodChannelResponseHandle* response_handle_new(
    FlBinaryMessengerResponseHandle* response_handle) {
  FlMethodChannelResponseHandle* handle =
      static_cast<FlMethodChannelResponseHandle*>(
          g_malloc0(sizeof(FlMethodChannelResponseHandle)));
  handle->response_handle = response_handle;

  return handle;
}

static void response_handle_free(FlMethodChannelResponseHandle* handle) {
  g_free(handle);
}

G_DEFINE_AUTOPTR_CLEANUP_FUNC(FlMethodChannelResponseHandle,
                              response_handle_free);

// Called when a binary message is received on this channel
static void message_cb(FlBinaryMessenger* messenger,
                       const gchar* channel,
                       GBytes* message,
                       FlBinaryMessengerResponseHandle* response_handle,
                       gpointer user_data) {
  FlMethodChannel* self = FL_METHOD_CHANNEL(user_data);

  if (self->method_call_handler == nullptr) {
    fl_method_channel_respond_not_implemented(
        self, response_handle_new(response_handle), nullptr);
    return;
  }

  g_autofree gchar* method = nullptr;
  g_autoptr(FlValue) args = nullptr;
  g_autoptr(GError) error = nullptr;
  if (!fl_method_codec_decode_method_call(self->codec, message, &method, &args,
                                          &error)) {
    g_warning("Failed to decode method call: %s", error->message);
    fl_method_channel_respond_not_implemented(
        self, response_handle_new(response_handle), nullptr);
    return;
  }

  self->method_call_handler(self, method, args,
                            response_handle_new(response_handle),
                            self->method_call_handler_data);
}

// Called when a response is received to a sent message
static void message_response_cb(GObject* object,
                                GAsyncResult* result,
                                gpointer user_data) {
  GTask* task = G_TASK(user_data);
  g_task_return_pointer(task, result, g_object_unref);
}

static void fl_method_channel_dispose(GObject* object) {
  FlMethodChannel* self = FL_METHOD_CHANNEL(object);

  if (self->messenger != nullptr)
    fl_binary_messenger_set_message_handler_on_channel(
        self->messenger, self->name, nullptr, nullptr);

  g_clear_object(&self->messenger);
  g_clear_pointer(&self->name, g_free);
  g_clear_object(&self->codec);

  G_OBJECT_CLASS(fl_method_channel_parent_class)->dispose(object);
}

static void fl_method_channel_class_init(FlMethodChannelClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_method_channel_dispose;
}

static void fl_method_channel_init(FlMethodChannel* self) {}

G_MODULE_EXPORT FlMethodChannel* fl_method_channel_new(
    FlBinaryMessenger* messenger,
    const gchar* name,
    FlMethodCodec* codec) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);
  g_return_val_if_fail(name != nullptr, nullptr);
  g_return_val_if_fail(FL_IS_METHOD_CODEC(codec), nullptr);

  FlMethodChannel* self =
      FL_METHOD_CHANNEL(g_object_new(fl_method_channel_get_type(), nullptr));

  self->messenger = FL_BINARY_MESSENGER(g_object_ref(messenger));
  self->name = g_strdup(name);
  self->codec = FL_METHOD_CODEC(g_object_ref(codec));

  fl_binary_messenger_set_message_handler_on_channel(
      self->messenger, self->name, message_cb, self);

  return self;
}

G_MODULE_EXPORT void fl_method_channel_set_method_call_handler(
    FlMethodChannel* self,
    FlMethodChannelMethodCallHandler handler,
    gpointer user_data) {
  g_return_if_fail(FL_IS_METHOD_CHANNEL(self));

  self->method_call_handler = handler;
  self->method_call_handler_data = user_data;
}

G_MODULE_EXPORT void fl_method_channel_invoke_method(
    FlMethodChannel* self,
    const gchar* method,
    FlValue* args,
    GCancellable* cancellable,
    GAsyncReadyCallback callback,
    gpointer user_data) {
  g_return_if_fail(FL_IS_METHOD_CHANNEL(self));
  g_return_if_fail(method != nullptr);

  g_autoptr(GTask) task =
      callback != nullptr ? g_task_new(self, cancellable, callback, user_data)
                          : nullptr;

  g_autoptr(GError) error = nullptr;
  g_autoptr(GBytes) message =
      fl_method_codec_encode_method_call(self->codec, method, args, &error);
  if (message == nullptr) {
    if (task != nullptr)
      g_task_return_error(task, error);
    return;
  }

  fl_binary_messenger_send_on_channel(
      self->messenger, self->name, message, cancellable,
      callback != nullptr ? message_response_cb : nullptr,
      g_steal_pointer(&task));
}

G_MODULE_EXPORT FlMethodResponse* fl_method_channel_invoke_method_finish(
    FlMethodChannel* self,
    GAsyncResult* result,
    GError** error) {
  g_return_val_if_fail(FL_IS_METHOD_CHANNEL(self), nullptr);
  g_return_val_if_fail(g_task_is_valid(result, self), nullptr);

  g_autoptr(GTask) task = G_TASK(result);
  GAsyncResult* r = G_ASYNC_RESULT(g_task_propagate_pointer(task, nullptr));

  g_autoptr(GBytes) response =
      fl_binary_messenger_send_on_channel_finish(self->messenger, r, error);
  if (response == nullptr)
    return nullptr;

  return fl_method_codec_decode_response(self->codec, response, error);
}

G_MODULE_EXPORT gboolean
fl_method_channel_respond(FlMethodChannel* self,
                          FlMethodChannelResponseHandle* response_handle,
                          FlValue* result,
                          GError** error) {
  g_return_val_if_fail(FL_IS_METHOD_CHANNEL(self), FALSE);
  g_return_val_if_fail(response_handle != nullptr, FALSE);

  // Take reference to ensure it is freed
  g_autoptr(FlMethodChannelResponseHandle) handle = response_handle;

  g_autoptr(GBytes) response =
      fl_method_codec_encode_success_envelope(self->codec, result, error);
  if (response == nullptr)
    return FALSE;

  return fl_binary_messenger_send_response(
      self->messenger, handle->response_handle, response, error);
}

G_MODULE_EXPORT gboolean
fl_method_channel_respond_error(FlMethodChannel* self,
                                FlMethodChannelResponseHandle* response_handle,
                                const gchar* code,
                                const gchar* message,
                                FlValue* details,
                                GError** error) {
  g_return_val_if_fail(FL_IS_METHOD_CHANNEL(self), FALSE);
  g_return_val_if_fail(response_handle != nullptr, FALSE);
  g_return_val_if_fail(code != nullptr, FALSE);

  // Take reference to ensure it is freed
  g_autoptr(FlMethodChannelResponseHandle) owned_response_handle =
      response_handle;

  g_autoptr(GBytes) response = fl_method_codec_encode_error_envelope(
      self->codec, code, message, details, error);
  if (response == nullptr)
    return FALSE;

  return fl_binary_messenger_send_response(
      self->messenger, owned_response_handle->response_handle, response, error);
}

G_MODULE_EXPORT gboolean fl_method_channel_respond_not_implemented(
    FlMethodChannel* self,
    FlMethodChannelResponseHandle* response_handle,
    GError** error) {
  g_return_val_if_fail(FL_IS_METHOD_CHANNEL(self), FALSE);
  g_return_val_if_fail(response_handle != nullptr, FALSE);

  // Take reference to ensure it is freed
  g_autoptr(FlMethodChannelResponseHandle) owned_response_handle =
      response_handle;

  return fl_binary_messenger_send_response(
      self->messenger, owned_response_handle->response_handle, nullptr, error);
}
