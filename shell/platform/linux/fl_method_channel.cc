// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_method_channel.h"

#include "flutter/shell/platform/linux/fl_method_codec_private.h"

#include <gmodule.h>

struct _FlMethodChannel {
  GObject parent_instance;

  FlBinaryMessenger* messenger;
  FlMethodCodec* codec;
  gchar* name;

  FlMethodChannelCallback callback;
  gpointer callback_data;
};

// Added here to stop the compiler from optimising this function away
G_MODULE_EXPORT GType fl_method_channel_get_type();

G_DEFINE_TYPE(FlMethodChannel, fl_method_channel, G_TYPE_OBJECT)

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

static void message_cb(FlBinaryMessenger* messenger,
                       const gchar* channel,
                       GBytes* message,
                       FlBinaryMessengerResponseHandle* response_handle,
                       gpointer user_data) {
  FlMethodChannel* self = FL_METHOD_CHANNEL(user_data);

  if (self->callback == nullptr) {
    fl_binary_messenger_send_response(messenger, response_handle, nullptr,
                                      nullptr);
    return;
  }

  g_autofree gchar* method = nullptr;
  g_autoptr(FlValue) args = nullptr;
  g_autoptr(GError) error = nullptr;
  if (!fl_method_codec_decode_method_call(self->codec, message, &method, &args,
                                          &error)) {
    g_warning("Failed to decode method call: %s", error->message);
    fl_binary_messenger_send_response(messenger, response_handle, nullptr,
                                      nullptr);
    return;
  }

  self->callback(self, method, args, response_handle_new(response_handle),
                 self->callback_data);
}

static void send_on_channel_ready_cb(GObject* object,
                                     GAsyncResult* result,
                                     gpointer user_data) {
  GTask* task = static_cast<GTask*>(user_data);
  g_task_return_pointer(task, result, g_object_unref);
}

static void fl_method_channel_dispose(GObject* object) {
  FlMethodChannel* self = FL_METHOD_CHANNEL(object);

  g_clear_pointer(&self->name, g_free);

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
  g_return_val_if_fail(name != nullptr, nullptr);
  g_return_val_if_fail(FL_IS_METHOD_CODEC(codec), nullptr);

  FlMethodChannel* self = static_cast<FlMethodChannel*>(
      g_object_new(fl_method_channel_get_type(), nullptr));
  self->messenger = static_cast<FlBinaryMessenger*>(g_object_ref(messenger));
  self->name = g_strdup(name);
  self->codec = static_cast<FlMethodCodec*>(g_object_ref(codec));

  fl_binary_messenger_set_message_handler_on_channel(messenger, name,
                                                     message_cb, self);

  return self;
}

G_MODULE_EXPORT void fl_method_channel_set_callback(
    FlMethodChannel* self,
    FlMethodChannelCallback callback,
    gpointer user_data) {
  g_return_if_fail(FL_IS_METHOD_CHANNEL(self));
  self->callback = callback;
  self->callback_data = user_data;
}

G_MODULE_EXPORT void fl_method_channel_invoke(FlMethodChannel* self,
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
      callback != nullptr ? send_on_channel_ready_cb : nullptr,
      g_steal_pointer(&task));
}

G_MODULE_EXPORT gboolean fl_method_channel_invoke_finish(FlMethodChannel* self,
                                                         GAsyncResult* result,
                                                         gchar** error_code,
                                                         gchar** error_message,
                                                         FlValue** value,
                                                         GError** error) {
  g_return_val_if_fail(FL_IS_METHOD_CHANNEL(self), FALSE);
  g_return_val_if_fail(g_task_is_valid(result, self), FALSE);
  g_return_val_if_fail(error_code, FALSE);

  g_autoptr(GTask) task = reinterpret_cast<GTask*>(result);
  GAsyncResult* r =
      static_cast<GAsyncResult*>(g_task_propagate_pointer(task, nullptr));

  g_autoptr(GBytes) response =
      fl_binary_messenger_send_on_channel_finish(self->messenger, r, error);
  if (response == nullptr)
    return FALSE;

  return fl_method_codec_decode_response(self->codec, response, error_code,
                                         error_message, value, error);
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

  gboolean result = fl_binary_messenger_send_response(
      self->messenger, owned_response_handle->response_handle, response, error);

  return result;
}
