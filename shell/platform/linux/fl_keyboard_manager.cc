// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_manager.h"

#include <cinttypes>

struct _FlKeyboardPendingEvent {
  GdkEventKey* event;

  // Self-incrementing ID attached to an event sent to the framework.
  uint64_t sequence_id;
  // The number of responders that haven't replied.
  size_t unreplied;
  // Whether any replied responders reported true (handled).
  bool any_handled;

  // A value calculated out of critical event information that can be used
  // to identify redispatched events.
  uint64_t hash;
};


// Calculates a unique ID for a given GdkEventKey object to use for
// identification of responses from the framework.
static uint64_t fl_keyboard_manager_get_event_hash(GdkEventKey* event) {
  // Combine the event timestamp, the type of event, and the hardware keycode
  // (scan code) of the event to come up with a unique id for this event that
  // can be derived solely from the event data itself, so that we can identify
  // whether or not we have seen this event already.
  return (event->time & 0xffffffff) |
         (static_cast<uint64_t>(event->type) & 0xffff) << 32 |
         (static_cast<uint64_t>(event->hardware_keycode) & 0xffff) << 48;
}

struct _FlKeyboardManager {
  GObject parent_instance;

  FlKeyboardManagerRedispatcher redispatch_callback;

  // A text plugin. Automatially released on dispose.
  FlTextInputPlugin* text_input_plugin;
  // An array of #FlKeyResponder. Elements are added with
  // #fl_keyboard_manager_add_responder immediately after initialization and are
  // automatically released on dispose.
  GPtrArray* responder_list;

  // An array of #_FlKeyboardPendingEvent. FlKeyboardManager must manually
  // release the elements unless it is transferring them to
  // pending_redispatches.
  GPtrArray* pending_responds;
  // An array of #_FlKeyboardPendingEvent. FlKeyboardManager must manually
  // release the elements.
  GPtrArray* pending_redispatches;

  uint64_t last_sequence_id;
};

G_DEFINE_TYPE(FlKeyboardManager,
              fl_keyboard_manager,
              G_TYPE_OBJECT);

// Disposes of an FlKeyboardManager instance.
static void fl_keyboard_manager_dispose(GObject* object) {
  FlKeyboardManager* self = FL_KEYBOARD_MANAGER(object);

  g_clear_object(&self->text_input_plugin);
  g_ptr_array_free(self->responder_list, TRUE);
  g_ptr_array_set_free_func(self->pending_responds, g_free);
  g_ptr_array_free(self->pending_responds, TRUE);
  g_ptr_array_set_free_func(self->pending_redispatches, g_free);
  g_ptr_array_free(self->pending_redispatches, TRUE);

  G_OBJECT_CLASS(fl_keyboard_manager_parent_class)->dispose(object);
}

// Initializes the FlKeyboardManager class methods.
static void fl_keyboard_manager_class_init(FlKeyboardManagerClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_manager_dispose;
}

static void fl_keyboard_manager_init(FlKeyboardManager* self) {
}

// Compare a #_FlKeyboardPendingEvent with the given sequence_id. The #needle
// should be a pointer to uint64_t sequence_id.
static gboolean compare_pending_by_sequence_id(gconstpointer pending, gconstpointer needle_sequence_id) {
  uint64_t sequence_id = *reinterpret_cast<const uint64_t*>(needle_sequence_id);
  return static_cast<const _FlKeyboardPendingEvent*>(pending)->sequence_id == sequence_id;
}

// Compare a #_FlKeyboardPendingEvent with the given hash. The #needle should be
// a pointer to uint64_t hash.
static gboolean compare_pending_by_hash(gconstpointer pending, gconstpointer needle_hash) {
  uint64_t hash = *reinterpret_cast<const uint64_t*>(needle_hash);
  return static_cast<const _FlKeyboardPendingEvent*>(pending)->hash == hash;
}

static bool fl_keyboard_manager_remove_redispatched(FlKeyboardManager* self, uint64_t hash) {
  guint result_index;
  gboolean found = g_ptr_array_find_with_equal_func(
    self->pending_redispatches,
    static_cast<const uint64_t*>(&hash),
    compare_pending_by_hash,
    &result_index);
  if (found) {
    gpointer removed = g_ptr_array_remove_index_fast(self->pending_redispatches, result_index);
    g_return_val_if_fail(removed != nullptr, TRUE);
    g_free(removed);
    return TRUE;
  } else {
    return FALSE;
  }
}

static void responder_handle_event_callback(FlKeyboardManager* self, uint64_t sequence_id, bool handled) {
  g_return_if_fail(FL_IS_KEYBOARD_MANAGER(self));

  guint result_index;
  gboolean found = g_ptr_array_find_with_equal_func(
    self->pending_responds,
    static_cast<const uint64_t*>(&sequence_id),
    compare_pending_by_sequence_id,
    &result_index);
  g_return_if_fail(found);
  _FlKeyboardPendingEvent* pending = static_cast<_FlKeyboardPendingEvent*>(g_ptr_array_index(self->pending_responds, result_index));
  g_return_if_fail(pending != nullptr);
  g_return_if_fail(pending->unreplied > 0);
  pending->unreplied -= 1;
  pending->any_handled = pending->any_handled || handled;
  // All responders have replied.
  if (pending->unreplied == 0) {
    g_ptr_array_remove_index_fast(self->pending_responds, result_index);
    bool should_redispatch = false;
    if (!pending->any_handled) {
      // If no responders have handled, send it to text plugin.
      if (!fl_text_input_plugin_filter_keypress(self->text_input_plugin, pending->event)) {
        // If text plugin doesn't handle either, redispatch.
        should_redispatch = true;
      }
    }
    if (should_redispatch) {
      g_ptr_array_add(self->pending_redispatches, pending);
      self->redispatch_callback(reinterpret_cast<GdkEvent*>(pending->event));
    } else {
      g_free(pending);
    }
  }
}

static void dispatch_pending_to_responder(gpointer responder_data, gpointer event_data) {
  FlKeyResponder* responder = FL_KEY_RESPONDER(responder_data);
  GdkEventKey* event = static_cast<GdkEventKey*>(event_data);
  fl_key_responder_handle_event(responder, event, responder_handle_event_callback, nullptr);
}

FlKeyboardManager* fl_keyboard_manager_new(
    FlTextInputPlugin* text_input_plugin,
    FlKeyboardManagerRedispatcher redispatch_callback) {
  g_return_val_if_fail(FL_IS_TEXT_INPUT_PLUGIN(text_input_plugin), nullptr);
  g_return_val_if_fail(redispatch_callback != nullptr, nullptr);

  FlKeyboardManager* self = FL_KEYBOARD_MANAGER(
      g_object_new(fl_keyboard_manager_get_type(), nullptr));

  self->text_input_plugin = text_input_plugin;
  self->redispatch_callback = redispatch_callback;
  self->responder_list = g_ptr_array_new_with_free_func(g_object_unref);

  self->pending_responds = g_ptr_array_new();
  self->pending_redispatches = g_ptr_array_new();

  self->last_sequence_id = 1;

  return self;
}

void fl_keyboard_manager_add_responder(
    FlKeyboardManager* self,
    FlKeyResponder* responder) {
  g_return_if_fail(FL_IS_KEYBOARD_MANAGER(self));
  g_return_if_fail(responder != nullptr);

  g_ptr_array_add(self->responder_list, responder);
}

gboolean fl_keyboard_manager_handle_event(FlKeyboardManager* self, GdkEventKey* event) {
  g_return_val_if_fail(FL_IS_KEYBOARD_MANAGER(self), FALSE);
  g_return_val_if_fail(event != nullptr, FALSE);

  uint64_t incoming_hash = fl_keyboard_manager_get_event_hash(event);
  if (fl_keyboard_manager_remove_redispatched(self, incoming_hash)) {
    return FALSE;
  }

  _FlKeyboardPendingEvent* pending = g_new(_FlKeyboardPendingEvent, 1);
  pending->event = reinterpret_cast<GdkEventKey*>(
      gdk_event_copy(reinterpret_cast<GdkEvent*>(event)));
  pending->sequence_id = (++self->last_sequence_id);
  pending->unreplied = self->responder_list->len;
  pending->any_handled = FALSE;
  pending->hash = incoming_hash;

  g_ptr_array_add(self->pending_responds, pending);
  g_ptr_array_foreach(self->responder_list, dispatch_pending_to_responder, self);

  return TRUE;
}
