// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_channel_responder.h"

#include <gtk/gtk.h>
#include <cinttypes>

#include "flutter/shell/platform/linux/fl_keyboard_manager.h"
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

// Definition of the FlKeyChannelResponder GObject class.

struct _FlKeyChannelResponder {
  GObject parent_instance;

  FlKeyboardManager* manager;
  FlBasicMessageChannel* channel;
  GPtrArray* pending_events;
  uint64_t last_id;
};

static void fl_key_channel_responder_iface_init(
    FlKeyResponderInterface* iface);

G_DEFINE_TYPE_WITH_CODE(
  FlKeyChannelResponder,
  fl_key_channel_responder,
  G_TYPE_OBJECT,
  G_IMPLEMENT_INTERFACE(FL_TYPE_KEY_RESPONDER,
                        fl_key_channel_responder_iface_init))

static bool fl_key_channel_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data);

static void fl_key_channel_responder_iface_init(
    FlKeyResponderInterface* iface) {
  iface->handle_event = fl_key_channel_responder_handle_event;
}

// Declare and define a private pair object to bind the id and the event
// together.

G_DECLARE_FINAL_TYPE(FlKeyEventPair,
                     fl_key_event_pair,
                     FL,
                     KEY_EVENT_PAIR,
                     GObject);

struct _FlKeyEventPair {
  GObject parent_instance;

  uint64_t id;
  GdkEventKey* event;
};

G_DEFINE_TYPE(FlKeyEventPair, fl_key_event_pair, G_TYPE_OBJECT)

// Dispose method for FlKeyEventPair.
static void fl_key_event_pair_dispose(GObject* object) {
  // Redundant, but added so that we don't get a warning about unused function
  // for FL_IS_KEY_EVENT_PAIR.
  g_return_if_fail(FL_IS_KEY_EVENT_PAIR(object));

  FlKeyEventPair* self = FL_KEY_EVENT_PAIR(object);
  g_clear_pointer(&self->event, gdk_event_free);
  G_OBJECT_CLASS(fl_key_event_pair_parent_class)->dispose(object);
}

// Class Initialization method for FlKeyEventPair class.
static void fl_key_event_pair_class_init(FlKeyEventPairClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_key_event_pair_dispose;
}

// Initialization for FlKeyEventPair instances.
static void fl_key_event_pair_init(FlKeyEventPair* self) {}

// Creates a new FlKeyEventPair instance, given a unique ID, and an event struct
// to keep.
FlKeyEventPair* fl_key_event_pair_new(uint64_t id, GdkEventKey* event) {
  FlKeyEventPair* self =
      FL_KEY_EVENT_PAIR(g_object_new(fl_key_event_pair_get_type(), nullptr));

  // Copy the event to preserve refcounts for referenced values (mainly the
  // window).
  GdkEventKey* event_copy = reinterpret_cast<GdkEventKey*>(
      gdk_event_copy(reinterpret_cast<GdkEvent*>(event)));
  self->id = id;
  self->event = event_copy;
  return self;
}

// Declare and define a private class to hold response data from the framework.
G_DECLARE_FINAL_TYPE(FlKeyEventResponseData,
                     fl_key_event_response_data,
                     FL,
                     KEY_EVENT_RESPONSE_DATA,
                     GObject);

struct _FlKeyEventResponseData {
  GObject parent_instance;

  FlKeyChannelResponder* responder;
  uint64_t id;
  FlKeyResponderAsyncCallback callback;
  gpointer user_data;
};

// Definition for FlKeyEventResponseData private class.
G_DEFINE_TYPE(FlKeyEventResponseData, fl_key_event_response_data, G_TYPE_OBJECT)

// Dispose method for FlKeyEventResponseData private class.
static void fl_key_event_response_data_dispose(GObject* object) {
  g_return_if_fail(FL_IS_KEY_EVENT_RESPONSE_DATA(object));
  FlKeyEventResponseData* self = FL_KEY_EVENT_RESPONSE_DATA(object);
  if (self->responder != nullptr) {
    g_object_remove_weak_pointer(G_OBJECT(self->responder),
                                 reinterpret_cast<gpointer*>(&(self->responder)));
    self->responder = nullptr;
  }
}

// Class initialization method for FlKeyEventResponseData private class.
static void fl_key_event_response_data_class_init(
    FlKeyEventResponseDataClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_key_event_response_data_dispose;
}

// Instance initialization method for FlKeyEventResponseData private class.
static void fl_key_event_response_data_init(FlKeyEventResponseData* self) {}

// Creates a new FlKeyEventResponseData private class with a responder that created
// the request, a unique ID for tracking, and optional user data.
// Will keep a weak pointer to the responder.
FlKeyEventResponseData* fl_key_event_response_data_new(FlKeyChannelResponder* responder,
                                                       uint64_t id,
                                                       FlKeyResponderAsyncCallback callback,
                                                       gpointer user_data) {
  FlKeyEventResponseData* self = FL_KEY_EVENT_RESPONSE_DATA(
      g_object_new(fl_key_event_response_data_get_type(), nullptr));

  self->responder = responder;
  // Add a weak pointer so we can know if the key event responder disappeared
  // while the framework was responding.
  g_object_add_weak_pointer(G_OBJECT(responder),
                            reinterpret_cast<gpointer*>(&(self->responder)));
  self->id = id;
  self->callback = callback;
  self->user_data = user_data;
  return self;
}

// Finds an event in the event queue that was sent to the framework by its ID.
GdkEventKey* fl_key_channel_responder_find_pending_event(FlKeyChannelResponder* self,
                                                    uint64_t id) {
  for (guint i = 0; i < self->pending_events->len; ++i) {
    if (FL_KEY_EVENT_PAIR(g_ptr_array_index(self->pending_events, i))->id ==
        id) {
      return FL_KEY_EVENT_PAIR(g_ptr_array_index(self->pending_events, i))
          ->event;
    }
  }
  return nullptr;
}

// Removes an event from the pending event queue.
static void remove_pending_event(FlKeyChannelResponder* self, uint64_t id) {
  for (guint i = 0; i < self->pending_events->len; ++i) {
    if (FL_KEY_EVENT_PAIR(g_ptr_array_index(self->pending_events, i))->id ==
        id) {
      g_ptr_array_remove_index(self->pending_events, i);
      return;
    }
  }
  g_warning("Tried to remove pending event with id %" PRIu64
            ", but the event was not found.",
            id);
}

// Adds an GdkEventKey to the pending event queue, with a unique ID, and the
// responder that added it.
static void add_pending_event(FlKeyChannelResponder* self,
                              uint64_t id,
                              GdkEventKey* event) {
  if (self->pending_events->len > kMaxPendingEvents) {
    g_warning(
        "There are %d keyboard events that have not yet received a "
        "response from the framework. Are responses being sent?",
        self->pending_events->len);
  }
  g_ptr_array_add(self->pending_events, fl_key_event_pair_new(id, event));
}

// Handles a response from the framework to a key event sent to the framework
// earlier.
static void handle_response(GObject* object,
                            GAsyncResult* result,
                            gpointer user_data) {
  g_autoptr(FlKeyEventResponseData) data =
      FL_KEY_EVENT_RESPONSE_DATA(user_data);

  // Will also return if the weak pointer has been destroyed.
  if (data->responder == nullptr) {
    return;
  }

  FlKeyChannelResponder* self = data->responder;

  g_autoptr(GError) error = nullptr;
  FlBasicMessageChannel* messageChannel = FL_BASIC_MESSAGE_CHANNEL(object);
  FlValue* message =
      fl_basic_message_channel_send_finish(messageChannel, result, &error);
  if (error != nullptr) {
    g_warning("Unable to retrieve framework response: %s", error->message);
    return;
  }
  g_autoptr(FlValue) handled_value = fl_value_lookup_string(message, "handled");
  bool handled = fl_value_get_bool(handled_value);

  remove_pending_event(self, data->id);
  data->callback(self->manager, data->id, handled);
}

// Disposes of an FlKeyChannelResponder instance.
static void fl_key_channel_responder_dispose(GObject* object) {
  FlKeyChannelResponder* self = FL_KEY_CHANNEL_RESPONDER(object);

  g_clear_object(&self->channel);
  g_ptr_array_free(self->pending_events, TRUE);

  G_OBJECT_CLASS(fl_key_channel_responder_parent_class)->dispose(object);
}

// Initializes the FlKeyChannelResponder class methods.
static void fl_key_channel_responder_class_init(FlKeyChannelResponderClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_key_channel_responder_dispose;
}

// Initializes an FlKeyChannelResponder instance.
static void fl_key_channel_responder_init(FlKeyChannelResponder* self) {}

// Creates a new FlKeyChannelResponder instance, with a messenger used to send
// messages to the framework, an FlTextInputPlugin used to handle key events
// that the framework doesn't handle. Mainly for testing purposes, it also takes
// an optional callback to call when a response is received, and an optional
// channel name to use when sending messages.
FlKeyChannelResponder* fl_key_channel_responder_new(
    FlKeyboardManager* manager,
    FlBinaryMessenger* messenger) {
  g_return_val_if_fail(FL_IS_BINARY_MESSENGER(messenger), nullptr);

  FlKeyChannelResponder* self = FL_KEY_CHANNEL_RESPONDER(g_object_new(FL_KEY_CHANNEL_RESPONDER(), nullptr));
  self->last_id = 1;
  self->manager = manager;

  g_autoptr(FlJsonMessageCodec) codec = fl_json_message_codec_new();
  self->channel = fl_basic_message_channel_new(
      messenger, kChannelName,
      FL_MESSAGE_CODEC(codec));

  self->pending_events = g_ptr_array_new_with_free_func(g_object_unref);
  return self;
}

// Sends a key event to the framework.
static bool fl_key_channel_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data) {
  FlKeyChannelResponder* self = FL_KEY_CHANNEL_RESPONDER(responder);
  g_return_val_if_fail(event != nullptr, FALSE);
  g_return_val_if_fail(callback != nullptr, FALSE);

  uint64_t id = (++self->last_id);

  const gchar* type;
  switch (event->type) {
    case GDK_KEY_PRESS:
      type = kTypeValueDown;
      break;
    case GDK_KEY_RELEASE:
      type = kTypeValueUp;
      break;
    default:
      return FALSE;
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

  static bool shift_lock_pressed = FALSE;
  static bool caps_lock_pressed = FALSE;
  static bool num_lock_pressed = FALSE;
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
  add_pending_event(self, id, event);
  FlKeyEventResponseData* data =
      fl_key_event_response_data_new(self, id, callback, user_data);
  // Send the message off to the framework for handling (or not).
  fl_basic_message_channel_send(self->channel, message, nullptr,
                                handle_response, data);
  // Return true before we know what the framework will do, because if it
  // doesn't handle the key, we'll re-dispatch it later.
  return TRUE;
}
