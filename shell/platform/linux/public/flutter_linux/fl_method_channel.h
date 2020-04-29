// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_METHOD_CHANNEL_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_METHOD_CHANNEL_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <gio/gio.h>
#include <glib-object.h>

#include "fl_binary_messenger.h"
#include "fl_method_codec.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlMethodChannel,
                     fl_method_channel,
                     FL,
                     METHOD_CHANNEL,
                     GObject)

/**
 * FlMethodChannel:
 *
 * #FlMethodChannel is an object that allows calling methods in Dart code.
 */

/**
 * FlMethodChannelResponseHandle:
 *
 * A handle used to respond to method calls.
 */
typedef struct _FlMethodChannelResponseHandle FlMethodChannelResponseHandle;

/**
 * FlMethodChannelCallback:
 * @channel: a #FlMethodChannel
 * @method: a method name
 * @args: arguments to the method
 * @response_handle: handle used to respond to the method call
 * @user_data: (closure): data provided when registering this callback
 *
 * Function called when a method call is received.
 */
typedef void (*FlMethodChannelCallback)(
    FlMethodChannel* channel,
    const gchar* method,
    FlValue* args,
    FlMethodChannelResponseHandle* response_handle,
    gpointer user_data);

/**
 * fl_method_channel_new:
 * @messenger: a #FlBinaryMessenger
 * @name: a channel name
 * @codec: the method codec
 *
 * Create a new method channel. @codec must match the codec used on the Dart end
 * of the channel.
 *
 * Returns: a new #FlMethodChannel.
 */
FlMethodChannel* fl_method_channel_new(FlBinaryMessenger* messenger,
                                       const gchar* name,
                                       FlMethodCodec* codec);

/**
 * fl_method_channel_set_callback:
 * @channel: a #FlMethodChannel
 * @callback: function to call when a method call is received on this channel
 * @user_data: (closure): user data to pass to @callback
 *
 * Set the function called when a method is called. Call
 * fl_method_channel_respond() when the method completes. Ownership of
 * #FlMethodChannelResponseHandle is transferred to the caller, and the call
 * must be responded to to avoid memory leaks.
 */
void fl_method_channel_set_callback(FlMethodChannel* channel,
                                    FlMethodChannelCallback callback,
                                    gpointer user_data);

/**
 * fl_method_channel_invoke:
 * @channel: a #FlMethodChannel
 * @method: the method to call
 * @args: (allow-none): arguments to the method, must match what the
 * #FlMethodCodec supports
 * @cancellable: (allow-none): a #GCancellable or %NULL
 * @callback: (scope async): a #GAsyncReadyCallback to call when the request is
 * satisfied
 * @user_data: (closure): user data to pass to @callback
 *
 * Call a method on this channel.
 */
void fl_method_channel_invoke(FlMethodChannel* channel,
                              const gchar* method,
                              FlValue* args,
                              GCancellable* cancellable,
                              GAsyncReadyCallback callback,
                              gpointer user_data);

/**
 * fl_method_channel_invoke_finish:
 * @channel: a #FlMethodChannel
 * @result:  #GAsyncResult
 * @error_code: (transfer full): location to write error code.
 * @error_message: (transfer full): location to write error message.
 * @result: (transfer full): location to write call response or error details if
 * an error.
 * @error: (allow-none): #GError location to store the error occurring, or %NULL
 * to ignore.
 *
 * Complete request started with fl_method_channel_invoke().
 *
 * Returns: %TRUE on success.
 */
gboolean fl_method_channel_invoke_finish(FlMethodChannel* channel,
                                         GAsyncResult* result,
                                         gchar** error_code,
                                         gchar** error_message,
                                         FlValue** value,
                                         GError** error);

/**
 * fl_method_channel_respond:
 * @channel: a #FlMethodChannel
 * @response_handle: handle that was provided in a #FlMethodChannelCallback
 * @result: (allow-none): value to respond with, must match what the
 * #FlMethodCodec supports
 * @error: (allow-none): #GError location to store the error occurring, or %NULL
 * to ignore
 *
 * Respond to a method call.
 *
 * Returns: %TRUE on success.
 */
gboolean fl_method_channel_respond(
    FlMethodChannel* channel,
    FlMethodChannelResponseHandle* response_handle,
    FlValue* result,
    GError** error);

/**
 * fl_method_channel_respond_error:
 * @channel: a #FlMethodChannel
 * @response_handle: handle that was provided in a #FlMethodChannelCallback
 * @code: error code
 * @message: (allow-none): error message
 * @details: (allow-none): details for the error
 * @error: (allow-none): #GError location to store the error occurring, or %NULL
 * to ignore
 *
 * Respond to a method call with an error.
 *
 * Returns: %TRUE on success.
 */
gboolean fl_method_channel_respond_error(
    FlMethodChannel* channel,
    FlMethodChannelResponseHandle* response_handle,
    const gchar* code,
    const gchar* message,
    FlValue* details,
    GError** error);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_METHOD_CHANNEL_H_
