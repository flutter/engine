// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_embedder_responder.h"

#include <gtk/gtk.h>
#include <cinttypes>

#include "flutter/shell/platform/linux/key_mapping.h"
#include "flutter/shell/platform/linux/fl_keyboard_manager.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_basic_message_embedder.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_json_message_codec.h"

struct _FlKeyEmbedderResponder {
  GObject parent_instance;

  FlEngine* engine;

  // Stores pressed keys, mapping their Flutter physical key to Flutter logical key.
  //
  // Both keys and values are directly stored uint64s.
  GHashTable* pressing_records;

  // A static map from XKB to Flutter's physical key code
  GHashTable* xkb_to_physical_key;

  // A static map from GTK keyval to Flutter's logical key code
  GHashTable* keyval_to_logical_key;

  gchar* character_to_free;
};

static void fl_key_embedder_responder_iface_init(FlKeyResponderInterface* iface);

G_DEFINE_TYPE_WITH_CODE(
    FlKeyEmbedderResponder,
    fl_key_embedder_responder,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(FL_TYPE_KEY_RESPONDER,
                          fl_key_embedder_responder_iface_init))

static void fl_key_embedder_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data);

static void fl_key_embedder_responder_iface_init(
    FlKeyResponderInterface* iface) {
  iface->handle_event = fl_key_embedder_responder_handle_event;
}

// Declare and define a private class to hold response data from the framework.
G_DECLARE_FINAL_TYPE(FlKeyEventResponseData,
                     fl_key_event_response_data,
                     FL,
                     KEY_EVENT_RESPONSE_DATA,
                     GObject);

struct _FlKeyEventResponseData {
  GObject parent_instance;

  FlKeyEmbedderResponder* responder;
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
    g_object_remove_weak_pointer(
        G_OBJECT(self->responder),
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

// Creates a new FlKeyEventResponseData private class with a responder that
// created the request, a unique ID for tracking, and optional user data. Will
// keep a weak pointer to the responder.
static FlKeyEventResponseData* fl_key_event_response_data_new(
    FlKeyEmbedderResponder* responder,
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

// Disposes of an FlKeyEmbedderResponder instance.
static void fl_key_embedder_responder_dispose(GObject* object) {
  FlKeyEmbedderResponder* self = FL_KEY_EMBEDDER_RESPONDER(object);

  g_clear_pointer(&self->pressing_records, g_hash_table_unref);
  g_clear_pointer(&self->xkb_to_physical_key, g_hash_table_unref);
  g_clear_pointer(&self->keyval_to_logical_key, g_hash_table_unref);
  if (self->character_to_free != nullptr) {
    g_free(self->character_to_free);
  }

  G_OBJECT_CLASS(fl_key_embedder_responder_parent_class)->dispose(object);
}

// Initializes the FlKeyEmbedderResponder class methods.
static void fl_key_embedder_responder_class_init(
    FlKeyEmbedderResponderClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_key_embedder_responder_dispose;
}

// Initializes an FlKeyEmbedderResponder instance.
static void fl_key_embedder_responder_init(FlKeyEmbedderResponder* self) {}

// Creates a new FlKeyEmbedderResponder instance, with a messenger used to send
// messages to the framework, an FlTextInputPlugin used to handle key events
// that the framework doesn't handle. Mainly for testing purposes, it also takes
// an optional callback to call when a response is received, and an optional
// embedder name to use when sending messages.
FlKeyEmbedderResponder* fl_key_embedder_responder_new(
    FlEngine* engine) {
  g_return_val_if_fail(FL_IS_ENGINE(engine), nullptr);

  FlKeyEmbedderResponder* self = FL_KEY_EMBEDDER_RESPONDER(
      g_object_new(fl_key_embedder_responder_get_type(), nullptr));

  self->engine = engine;
  // Add a weak pointer so we can know if the key event responder disappeared
  // while the framework was responding.
  g_object_add_weak_pointer(G_OBJECT(engine),
                            reinterpret_cast<gpointer*>(&(self->engine)));

  self->pressing_records = g_hash_table_new(g_direct_hash, g_direct_equal);
  self->xkb_to_physical_key = g_hash_table_new(g_direct_hash, g_direct_equal);
  initialize_xkb_to_physical_key(self->xkb_to_physical_key);
  self->keyval_to_logical_key = g_hash_table_new(g_direct_hash, g_direct_equal);
  initialize_gtk_keyval_to_logical_key(self->keyval_to_logical_key);
  self->character_to_free = nullptr;

  return self;
}

static uint64_t event_to_physical_key(const GdkEventKey* event, GHashTable* table) {
  gpointer record = g_hash_table_lookup(table, GUINT_TO_POINTER(event->hardware_keycode));
  if (record != nullptr) {
    return GPOINTER_TO_UINT(record);
  }
  // Auto-generate key
  return kAutogeneratedMask | kLinuxKeyIdPlane | event->hardware_keycode;
}

static uint64_t event_to_logical_key(const GdkEventKey* event, GHashTable* table) {
  guint keyval = event->keyval;
  gpointer record = g_hash_table_lookup(table, GUINT_TO_POINTER(keyval));
  if (record != nullptr) {
    return GPOINTER_TO_UINT(record);
  }
  // ASCII // TODO
  if (keyval < 256) {
    return keyval;
  }
  // Auto-generate key
  return kAutogeneratedMask | kLinuxKeyIdPlane | keyval;
}

static uint64_t event_to_timestamp(const GdkEventKey* event) {
  return kMicrosecondsPerMillisecond * (double)event->time;
}

// Returns a newly accocated UTF-8 string from event->keyval that must be
// freed later with g_free().
static char* event_to_character(const GdkEventKey* event) {
  gunichar unicodeChar = gdk_keyval_to_unicode(event->keyval);
  glong items_written;
  gchar* result = g_ucs4_to_utf8(&unicodeChar, 1, NULL, &items_written, NULL);
  if (items_written == 0) {
    if (result != NULL)
      g_free(result);
    return nullptr;
  }
  return result;
}

static uint64_t gpointerToUint64(gpointer pointer) {
  return pointer == nullptr ? 0 : reinterpret_cast<uint64_t>(pointer);
}

static gpointer uint64ToGpointer(uint64_t number) {
  return reinterpret_cast<gpointer>(number);
}

// Return the logical key corresponding to the physical key.
//
// Returns 0 if not found.
static uint64_t pressed_logical_for_physical(GHashTable* pressing_records, uint64_t physical) {
  return gpointerToUint64(g_hash_table_lookup(pressing_records, uint64ToGpointer(physical)));
}

static size_t fl_keyboard_manager_convert_key_event(FlKeyboardManager* self,
                                             const GdkEventKey* event,
                                             FlKeyDatum* results) {
}

// Handles a response from the framework to a key event sent to the framework
// earlier.
static void handle_response(bool handled,
                            gpointer user_data) {
  g_autoptr(FlKeyEventResponseData) data =
      FL_KEY_EVENT_RESPONSE_DATA(user_data);

  // Return if the weak pointer has been destroyed.
  if (data->responder == nullptr) {
    return;
  }

  // Return if callback is not requested (happens for synthesized events).
  if (data->callback == nullptr) {
    return;
  }

  data->callback(handled, data->user_data);
}

// Sends a key event to the framework.
static void fl_key_embedder_responder_handle_event(
    FlKeyResponder* responder,
    GdkEventKey* event,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data) {
  FlKeyEmbedderResponder* self = FL_KEY_EMBEDDER_RESPONDER(responder);
  g_return_if_fail(event != nullptr);
  g_return_if_fail(callback != nullptr);

  printf("===START=== state %d\n", event->state);
  if (self->character_to_free != nullptr) {
    g_free(self->character_to_free);
    self->character_to_free = nullptr;
  }
  uint64_t physical_key = event_to_physical_key(event, self->xkb_to_physical_key);
  uint64_t logical_key = event_to_logical_key(event, self->keyval_to_logical_key);
  bool is_physical_down = event->type == GDK_KEY_PRESS;

  uint64_t last_logical_record = pressed_logical_for_physical(self->pressing_records, physical_key);
  uint64_t next_logical_record = is_physical_down ? last_logical_record : 0;

  char* character_to_free = nullptr;

  printf("last %lu next %lu down %d type %d\n", last_logical_record, next_logical_record, is_physical_down, event->type);
  fflush(stdout);

  FlutterKeyEvent event;
  event->struct_size = sizeof(event);
  event->type = kFlutterKeyEventTypeDown;
  event->physical = physical_key;
  event->logical = physical_key;
  event->character = character_to_free;
  event->synthesized = false;

  base_event->timestamp = event_to_timestamp(event);
  base_event->synthesized = false;

  if (is_physical_down) {
    character_to_free = event_to_character(event); // Might be null
    base_event->character = character_to_free;

    if (last_logical_record) {
      // GTK doesn't report repeat events separatedly, therefore we can't
      // distinguish a repeat event from a down event after a missed up event.
      base_event->kind = kFlKeyDataKindRepeat;
      base_event->logical = last_logical_record;
    } else {
      base_event->kind = kFlKeyDataKindDown;
      base_event->logical = logical_key;
    }
  } else { // is_physical_down false
    base_event->character = nullptr;
    base_event->kind = kFlKeyDataKindUp;
    base_event->logical = logical_key;
  }

  FlKeyEventResponseData* response_data = fl_key_event_response_data_new(
    self, callback, user_data);

  fl_engine_send_key_event(self->engine, &event, handle_response, response_data);

  return 1;
}
