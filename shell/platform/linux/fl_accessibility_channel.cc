// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_accessibility_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_basic_message_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_standard_message_codec.h"

struct _FlAccessibilityChannel {
  GObject parent_instance;

  FlBasicMessageChannel* channel;
};

G_DEFINE_TYPE(FlAccessibilityChannel, fl_accessibility_channel, G_TYPE_OBJECT)

// Called when a message is received on this channel
static void message_cb(FlBasicMessageChannel* channel,
                       FlValue* message,
                       FlBasicMessageChannelResponseHandle* response_handle,
                       gpointer user_data) {
  if (fl_value_get_type(message) != FL_VALUE_TYPE_MAP) {
    g_warning("Ignoring unknown flutter/accessibility message type");
    fl_basic_message_channel_respond(channel, response_handle, nullptr,
                                     nullptr);
    return;
  }

  FlValue* type_value = fl_value_lookup_string(message, "type");
  if (type_value == nullptr ||
      fl_value_get_type(type_value) != FL_VALUE_TYPE_STRING) {
    g_warning(
        "Ignoring unknown flutter/accessibility message with unknown type");
    fl_basic_message_channel_respond(channel, response_handle, nullptr,
                                     nullptr);
    return;
  }
  const gchar* type = fl_value_get_string(type_value);

  if (strcmp(type, "tooltip") == 0) {
    g_debug("Got tooltip");
    // FIXME: Make a callback
    fl_basic_message_channel_respond(channel, response_handle, nullptr,
                                     nullptr);
  } else {
    g_debug("Got unknown accessibility message: %s", type);
    fl_basic_message_channel_respond(channel, response_handle, nullptr,
                                     nullptr);
  }
}

static void fl_accessibility_channel_dispose(GObject* object) {
  FlAccessibilityChannel* self = FL_ACCESSIBILITY_CHANNEL(object);

  g_clear_object(&self->channel);

  G_OBJECT_CLASS(fl_accessibility_channel_parent_class)->dispose(object);
}

static void fl_accessibility_channel_class_init(
    FlAccessibilityChannelClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_accessibility_channel_dispose;
}

static void fl_accessibility_channel_init(FlAccessibilityChannel* self) {}

FlAccessibilityChannel* fl_accessibility_channel_new(
    FlBinaryMessenger* messenger) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);

  FlAccessibilityChannel* self = FL_ACCESSIBILITY_CHANNEL(
      g_object_new(fl_accessibility_channel_get_type(), nullptr));

  g_autoptr(FlStandardMessageCodec) codec = fl_standard_message_codec_new();
  self->channel = fl_basic_message_channel_new(
      messenger, "flutter/accessibility", FL_MESSAGE_CODEC(codec));
  fl_basic_message_channel_set_message_handler(self->channel, message_cb, self,
                                               nullptr);

  return self;
}
