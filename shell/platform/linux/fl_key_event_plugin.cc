// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_event_plugin.h"

#include <gtk/gtk.h>
#include <deque>

#include "flutter/shell/platform/linux/fl_text_input_plugin.h"
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

static constexpr uint64_t kMaxPendingEvents = 1000;

struct _FlKeyEventPlugin {
  GObject parent_instance;

  FlBasicMessageChannel* channel = nullptr;
  FlTextInputPlugin* text_input_plugin = nullptr;
  FlKeyEventPluginCallback response_callback = nullptr;

  std::deque<std::pair<uint64_t, GdkEventKey*>> pendingEvents;
};

G_DEFINE_TYPE(FlKeyEventPlugin, fl_key_event_plugin, G_TYPE_OBJECT)

static void fl_key_event_plugin_dispose(GObject* object) {
  FlKeyEventPlugin* self = FL_KEY_EVENT_PLUGIN(object);

  g_clear_object(&self->channel);
  g_object_remove_weak_pointer(
      G_OBJECT(self->text_input_plugin),
      reinterpret_cast<gpointer*>(&(self->text_input_plugin)));

  G_OBJECT_CLASS(fl_key_event_plugin_parent_class)->dispose(object);
}

static void fl_key_event_plugin_class_init(FlKeyEventPluginClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_key_event_plugin_dispose;
}

static void fl_key_event_plugin_init(FlKeyEventPlugin* self) {}

FlKeyEventPlugin* fl_key_event_plugin_new(
    FlBinaryMessenger* messenger,
    FlTextInputPlugin* text_input_plugin,
    FlKeyEventPluginCallback response_callback,
    const char* channel_name) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);
  g_return_val_if_fail(FL_IS_TEXT_INPUT_PLUGIN(text_input_plugin), nullptr);

  FlKeyEventPlugin* self = FL_KEY_EVENT_PLUGIN(
      g_object_new(fl_key_event_plugin_get_type(), nullptr));

  g_autoptr(FlJsonMessageCodec) codec = fl_json_message_codec_new();
  self->channel = fl_basic_message_channel_new(
      messenger, channel_name == nullptr ? kChannelName : channel_name,
      FL_MESSAGE_CODEC(codec));
  self->response_callback = response_callback;
  // Add a weak pointer so we know if the text input plugin goes away.
  g_object_add_weak_pointer(
      G_OBJECT(text_input_plugin),
      reinterpret_cast<gpointer*>(&(self->text_input_plugin)));
  self->text_input_plugin = text_input_plugin;

  return self;
}

uint64_t fl_get_event_id(GdkEventKey* event) {
  // Combine the event timestamp, the type of event, and the hardware keycode
  // (scan code) of the event to come up with a unique id for this event that
  // can be derived solely from the event data itself, so that we can identify
  // whether or not we have seen this event already.
  return (event->time & 0xffffffff) |
         (static_cast<uint64_t>(event->type) & 0xffff) << 32 |
         (static_cast<uint64_t>(event->hardware_keycode) & 0xffff) << 48;
}

GdkEventKey* fl_find_pending_event(FlKeyEventPlugin* self, uint64_t id) {
  if (self->pendingEvents.empty() || self->pendingEvents.front().first != id) {
    return nullptr;
  }
  return self->pendingEvents.front().second;
}

void fl_remove_pending_event(FlKeyEventPlugin* self, uint64_t id) {
  if (self->pendingEvents.empty() || self->pendingEvents.front().first != id) {
    g_warning(
        "Tried to remove pending event with id %ld, but the event was out of "
        "order, or is unknown.",
        id);
    return;
  }
  gdk_event_free(
      reinterpret_cast<GdkEvent*>(self->pendingEvents.front().second));
  self->pendingEvents.pop_front();
}

void fl_add_pending_event(FlKeyEventPlugin* self,
                          uint64_t id,
                          GdkEventKey* event) {
  if (self->pendingEvents.size() > kMaxPendingEvents) {
    g_warning(
        "There are %ld keyboard events that have not yet received a "
        "response from the framework. Are responses being sent?",
        self->pendingEvents.size());
  }
  // Copy the event to preserve refcounts for referenced values (mainly the
  // window).
  GdkEventKey* event_copy = reinterpret_cast<GdkEventKey*>(
      gdk_event_copy(reinterpret_cast<GdkEvent*>(event)));
  self->pendingEvents.push_back(std::make_pair(id, event_copy));
}

struct _KeyEventResponseData {
  FlKeyEventPlugin* self;
  uint64_t id;
  gpointer user_data;
};

void fl_handle_response(GObject* object,
                        GAsyncResult* result,
                        gpointer user_data) {
  _KeyEventResponseData* data =
      reinterpret_cast<_KeyEventResponseData*>(user_data);

  // Will also return if the weak pointer has been destroyed.
  g_return_if_fail(FL_IS_KEY_EVENT_PLUGIN(data->self));

  FlKeyEventPlugin* self = data->self;
  // Don't need to weak pointer anymore.
  g_object_remove_weak_pointer(G_OBJECT(self),
                               reinterpret_cast<gpointer*>(&(data->self)));

  g_autoptr(GError) error = nullptr;
  FlBasicMessageChannel* messageChannel = FL_BASIC_MESSAGE_CHANNEL(object);
  FlValue* message =
      fl_basic_message_channel_send_finish(messageChannel, result, &error);
  if (error != nullptr) {
    g_error("Unable to retrieve framework response: %s", error->message);
    g_error_free(error);
    return;
  }
  g_autoptr(FlValue) handled_value = fl_value_lookup_string(message, "handled");
  bool handled = false;
  if (handled_value != nullptr) {
    GdkEventKey* event = fl_find_pending_event(self, data->id);
    if (event == nullptr) {
      g_warning(
          "Event response for event id %ld received, but event was received "
          "out of order, or is unknown.",
          data->id);
    } else {
      handled = fl_value_get_bool(handled_value);
      if (!handled) {
        if (self->text_input_plugin != nullptr) {
          // Propagate the event to the text input plugin.
          handled = fl_text_input_plugin_filter_keypress(
              self->text_input_plugin, event);
        }
        // Dispatch the event to other GTK windows if the text input plugin
        // didn't handle it. We keep track of the event id so we can recognize
        // the event when our window receives it again and not respond to it. If
        // the response callback is set, then use that instead.
        if (!handled && self->response_callback == nullptr) {
          gdk_event_put(reinterpret_cast<GdkEvent*>(event));
        }
      }
    }
  }

  if (handled) {
    // Because the event was handled, we no longer need to track it. Unhandled
    // events will be removed when the event is re-dispatched to the window.
    fl_remove_pending_event(self, data->id);
  }

  if (self->response_callback != nullptr) {
    self->response_callback(object, message, handled, data->user_data);
  }
}

bool fl_key_event_plugin_send_key_event(FlKeyEventPlugin* self,
                                        GdkEventKey* event,
                                        gpointer user_data) {
  g_return_val_if_fail(FL_IS_KEY_EVENT_PLUGIN(self), false);
  g_return_val_if_fail(event != nullptr, false);

  // Get an ID for the event, so we can match them up when we get a response
  // from the framework. Use the event time, type, and hardware keycode as a
  // unique ID, since they are part of the event structure that we can look up
  // when we receive a random event that may or may not have been
  // tracked/produced by this code.
  uint64_t id = fl_get_event_id(event);
  if (!self->pendingEvents.empty() && self->pendingEvents.front().first == id) {
    // If the event is at the head of the queue of pending events we've seen,
    // and has the same id, then we know that this is a re-dispatched event, and
    // we shouldn't respond to it, but we should remove it from tracking.
    fl_remove_pending_event(self, id);
    return false;
  }

  const gchar* type;
  switch (event->type) {
    case GDK_KEY_PRESS:
      type = kTypeValueDown;
      break;
    case GDK_KEY_RELEASE:
      type = kTypeValueUp;
      break;
    default:
      return false;
  }

  int64_t scan_code = event->hardware_keycode;
  int64_t unicodeScalarValues = gdk_keyval_to_unicode(event->keyval);

  // For most modifier keys, GTK keeps track of the "pressed" state of the
  // modifier keys. Flutter uses this information to keep modifier keys from
  // being "stuck" when a key-up event is lost because it happens after the app
  // loses focus.
  //
  // For Lock keys (ShiftLock, CapsLock, NumLock), however, GTK keeps track of
  // the state of the locks themselves, not the "pressed" state of the key.
  //
  // Since Flutter expects the "pressed" state of the modifier keys, the lock
  // state for these keys is discarded here, and it is substituted for the
  // pressed state of the key.
  //
  // This code has the flaw that if a key event is missed due to the app losing
  // focus, then this state will still think the key is pressed when it isn't,
  // but that is no worse than for "regular" keys until we implement the
  // sync/cancel events on app focus changes.
  //
  // This is necessary to do here instead of in the framework because Flutter
  // does modifier key syncing in the framework, and will turn on/off these keys
  // as being "pressed" whenever the lock is on, which breaks a lot of
  // interactions (for example, if shift-lock is on, tab traversal is broken).
  //
  // TODO(gspencergoog): get rid of this tracked state when we are tracking the
  // state of all keys and sending sync/cancel events when focus is gained/lost.

  // Remove lock states from state mask.
  guint state = event->state & ~(GDK_LOCK_MASK | GDK_MOD2_MASK);

  static bool shift_lock_pressed = false;
  static bool caps_lock_pressed = false;
  static bool num_lock_pressed = false;
  switch (event->keyval) {
    case GDK_KEY_Num_Lock:
      num_lock_pressed = event->type == GDK_KEY_PRESS;
      break;
    case GDK_KEY_Caps_Lock:
      caps_lock_pressed = event->type == GDK_KEY_PRESS;
      break;
    case GDK_KEY_Shift_Lock:
      shift_lock_pressed = event->type == GDK_KEY_PRESS;
      break;
  }

  // Add back in the state matching the actual pressed state of the lock keys,
  // not the lock states.
  state |= (shift_lock_pressed || caps_lock_pressed) ? GDK_LOCK_MASK : 0x0;
  state |= num_lock_pressed ? GDK_MOD2_MASK : 0x0;

  g_autoptr(FlValue) message = fl_value_new_map();
  fl_value_set_string_take(message, kTypeKey, fl_value_new_string(type));
  fl_value_set_string_take(message, kKeymapKey,
                           fl_value_new_string(kLinuxKeymap));
  fl_value_set_string_take(message, kScanCodeKey, fl_value_new_int(scan_code));
  fl_value_set_string_take(message, kToolkitKey,
                           fl_value_new_string(kGtkToolkit));
  fl_value_set_string_take(message, kKeyCodeKey,
                           fl_value_new_int(event->keyval));
  fl_value_set_string_take(message, kModifiersKey, fl_value_new_int(state));
  if (unicodeScalarValues != 0) {
    fl_value_set_string_take(message, kUnicodeScalarValuesKey,
                             fl_value_new_int(unicodeScalarValues));
  }

  // Track the event as pending a response from the framework.
  fl_add_pending_event(self, id, event);
  _KeyEventResponseData* data = new _KeyEventResponseData{self, id, user_data};
  // Add a weak pointer so we can know if the key event plugin disappeared
  // while the framework was responding.
  g_object_add_weak_pointer(G_OBJECT(self),
                            reinterpret_cast<gpointer*>(&(data->self)));
  // Send the message off to the framework for handling (or not).
  fl_basic_message_channel_send(self->channel, message, nullptr,
                                fl_handle_response, data);
  // Return true before we know what the framework will do, because if it
  // doesn't handle the key, we'll re-dispatch it later.
  return true;
}
