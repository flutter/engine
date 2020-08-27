// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_event_plugin.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_basic_message_channel.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_json_message_codec.h"

static constexpr char kChannelName[] = "flutter/keyevent";
static constexpr char kTypeKey[] = "type";
static constexpr char kTypeValueUp[] = "keyup";
static constexpr char kTypeValueDown[] = "keydown";
static constexpr char kKeymapKey[] = "keymap";
static constexpr char kKeyCodeKey[] = "keyCode";
static constexpr char kScanCodeKey[] = "scanCode";
static constexpr char kModifiersKey[] = "modifiers";
static constexpr char kToolkitKey[] = "toolkit";
static constexpr char kUnicodeScalarValuesKey[] = "unicodeScalarValues";

static constexpr char kGtkToolkit[] = "gtk";
static constexpr char kLinuxKeymap[] = "linux";

struct _FlKeyEventPlugin {
  GObject parent_instance;

  FlBasicMessageChannel* channel;
};

G_DEFINE_TYPE(FlKeyEventPlugin, fl_key_event_plugin, G_TYPE_OBJECT)

static void fl_key_event_plugin_dispose(GObject* object) {
  FlKeyEventPlugin* self = FL_KEY_EVENT_PLUGIN(object);

  g_clear_object(&self->channel);

  G_OBJECT_CLASS(fl_key_event_plugin_parent_class)->dispose(object);
}

static void fl_key_event_plugin_class_init(FlKeyEventPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_key_event_plugin_dispose;
}

static void fl_key_event_plugin_init(FlKeyEventPlugin* self) {}

FlKeyEventPlugin* fl_key_event_plugin_new(FlBinaryMessenger* messenger) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);

  FlKeyEventPlugin* self = FL_KEY_EVENT_PLUGIN(
      g_object_new(fl_key_event_plugin_get_type(), nullptr));

  g_autoptr(FlJsonMessageCodec) codec = fl_json_message_codec_new();
  self->channel = fl_basic_message_channel_new(messenger, kChannelName,
                                               FL_MESSAGE_CODEC(codec));

  return self;
}

void fl_key_event_plugin_send_key_event(FlKeyEventPlugin* self,
                                        GdkEventKey* event) {
  g_return_if_fail(FL_IS_KEY_EVENT_PLUGIN(self));
  g_return_if_fail(event != nullptr);

  const gchar* type;
  if (event->type == GDK_KEY_PRESS)
    type = kTypeValueDown;
  else if (event->type == GDK_KEY_RELEASE)
    type = kTypeValueUp;
  else
    return;

  int64_t scan_code = event->hardware_keycode;
  int64_t unicodeScalarValues = gdk_keyval_to_unicode(event->keyval);

  // Remove lock states from state mask.
  guint state = event->state & ~(GDK_LOCK_MASK | GDK_MOD2_MASK);

  // GTK keeps track of the state of the locks themselves, not the "down" state
  // of the key. Flutter expects the "down" state of the modifier key, so we
  // keep track of the down state here, and send it to the framework. This code
  // has the flaw that if a key event is missed due to the app losing focus,
  // then this state will still think the key is down when it isn't, but that is
  // no worse than for other keys until we implement the sync/cancel events.
  //
  // This is necessary to do here instead of in the framework because Flutter
  // does modifier key syncing in the framework, and will turn on/off these keys
  // as being "pressed" whenever the lock is on, which breaks a lot of
  // interactions.
  //
  // TODO(gspencergoog): get rid of this tracked state when we are tracking the
  // state of all keys and sending sync/cancel events when focus is gained/lost.
  static bool shift_lock_down = false;
  static bool caps_lock_down = false;
  static bool num_lock_down = false;
  if (event->keyval == GDK_KEY_Num_Lock) {
    num_lock_down = event->type == GDK_KEY_PRESS;
  }
  if (event->keyval == GDK_KEY_Caps_Lock) {
    caps_lock_down = event->type == GDK_KEY_PRESS;
  }
  if (event->keyval == GDK_KEY_Shift_Lock) {
    shift_lock_down = event->type == GDK_KEY_PRESS;
  }
  // Make the state match the actual pressed state of the lock keys, not the
  // lock states themselves.
  state |= (shift_lock_down || caps_lock_down) ? GDK_LOCK_MASK : 0x0;
  state |= num_lock_down ? GDK_MOD2_MASK : 0x0;

  g_autoptr(FlValue) message = fl_value_new_map();
  fl_value_set_string_take(message, kTypeKey, fl_value_new_string(type));
  fl_value_set_string_take(message, kKeymapKey,
                           fl_value_new_string(kLinuxKeymap));
  fl_value_set_string_take(message, kScanCodeKey, fl_value_new_int(scan_code));
  fl_value_set_string_take(message, kToolkitKey,
                           fl_value_new_string(kGtkToolkit));
  fl_value_set_string_take(message, kKeyCodeKey,
                           fl_value_new_int(event->keyval));
  fl_value_set_string_take(message, kModifiersKey,
                           fl_value_new_int(state));
  if (unicodeScalarValues != 0) {
    fl_value_set_string_take(message, kUnicodeScalarValuesKey,
                             fl_value_new_int(unicodeScalarValues));
  }

  fl_basic_message_channel_send(self->channel, message, nullptr, nullptr,
                                nullptr);
}