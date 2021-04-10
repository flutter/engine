// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_manager.h"

#include <cinttypes>

// Declare and define a private class to hold response data from the framework.
#define FL_TYPE_KEYBOARD_MANAGER_USER_DATA \
  fl_keyboard_manager_user_data_get_type()
G_DECLARE_FINAL_TYPE(FlKeyboardManagerUserData,
                     fl_keyboard_manager_user_data,
                     FL,
                     KEYBOARD_MANAGER_USER_DATA,
                     GObject);

struct _FlKeyboardManagerUserData {
  GObject parent_instance;

  FlKeyboardManager* manager;
  uint64_t sequence_id;
};

namespace {
typedef struct {
  GdkEventKey* event;
  FlKeyboardManagerUserData* user_data;
} DispatchPendingToResponderForeachData;
}  // namespace

// Definition for FlKeyboardManagerUserData private class.
G_DEFINE_TYPE(FlKeyboardManagerUserData,
              fl_keyboard_manager_user_data,
              G_TYPE_OBJECT)

// Dispose method for FlKeyboardManagerUserData private class.
static void fl_keyboard_manager_user_data_dispose(GObject* object) {
  g_return_if_fail(FL_IS_KEYBOARD_MANAGER_USER_DATA(object));
  FlKeyboardManagerUserData* self = FL_KEYBOARD_MANAGER_USER_DATA(object);
  if (self->manager != nullptr) {
    g_object_remove_weak_pointer(G_OBJECT(self->manager),
                                 reinterpret_cast<gpointer*>(&(self->manager)));
    self->manager = nullptr;
  }
}

// Class initialization method for FlKeyboardManagerUserData private class.
static void fl_keyboard_manager_user_data_class_init(
    FlKeyboardManagerUserDataClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_manager_user_data_dispose;
}

// Instance initialization method for FlKeyboardManagerUserData private class.
static void fl_keyboard_manager_user_data_init(
    FlKeyboardManagerUserData* self) {}

// Creates a new FlKeyboardManagerUserData private class with a responder that
// created the request, a unique ID for tracking, and optional user data. Will
// keep a weak pointer to the responder.
FlKeyboardManagerUserData* fl_keyboard_manager_user_data_new(
    FlKeyboardManager* manager,
    uint64_t sequence_id) {
  FlKeyboardManagerUserData* self = FL_KEYBOARD_MANAGER_USER_DATA(
      g_object_new(fl_keyboard_manager_user_data_get_type(), nullptr));

  self->manager = manager;
  // Add a weak pointer so we can know if the key event responder disappeared
  // while the framework was responding.
  g_object_add_weak_pointer(G_OBJECT(manager),
                            reinterpret_cast<gpointer*>(&(self->manager)));
  self->sequence_id = sequence_id;
  return self;
}

G_DECLARE_FINAL_TYPE(FlKeyboardPendingEvent,
                     fl_keyboard_pending_event,
                     FL,
                     KEYBOARD_PENDING_EVENT,
                     GObject);

struct _FlKeyboardPendingEvent {
  GObject parent_instance;

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

G_DEFINE_TYPE(FlKeyboardPendingEvent, fl_keyboard_pending_event, G_TYPE_OBJECT)

// Dispose method for FlKeyboardPendingEvent.
static void fl_keyboard_pending_event_dispose(GObject* object) {
  // Redundant, but added so that we don't get a warning about unused function
  // for FL_IS_KEYBOARD_PENDING_EVENT.
  g_return_if_fail(FL_IS_KEYBOARD_PENDING_EVENT(object));

  FlKeyboardPendingEvent* self = FL_KEYBOARD_PENDING_EVENT(object);
  g_clear_pointer(&self->event, gdk_event_free);
  G_OBJECT_CLASS(fl_keyboard_pending_event_parent_class)->dispose(object);
}

// Class Initialization method for FlKeyboardPendingEvent class.
static void fl_keyboard_pending_event_class_init(
    FlKeyboardPendingEventClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_pending_event_dispose;
}

// Initialization for FlKeyboardPendingEvent instances.
static void fl_keyboard_pending_event_init(FlKeyboardPendingEvent* self) {}

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

FlKeyboardPendingEvent* fl_keyboard_pending_event_new(GdkEventKey* event,
                                                      uint64_t sequence_id,
                                                      size_t to_reply) {
  printf("new pending 1\n");
  FlKeyboardPendingEvent* self = FL_KEYBOARD_PENDING_EVENT(
      g_object_new(fl_keyboard_pending_event_get_type(), nullptr));
  printf("new pending 2\n");

  FL_KEYBOARD_PENDING_EVENT(self);

  printf("new 1\n");
  // Copy the event to preserve refcounts for referenced values (mainly the
  // window).
  GdkEventKey* event_copy = reinterpret_cast<GdkEventKey*>(
      gdk_event_copy(reinterpret_cast<GdkEvent*>(event)));
  printf("new 1.5\n");
  FlKeyboardPendingEvent* self2 = FL_KEYBOARD_PENDING_EVENT(self);
  printf("new 2\n");
  self->event = event_copy;
  self->sequence_id = sequence_id;
  self->unreplied = to_reply;
  self->any_handled = false;
  self->hash = fl_keyboard_manager_get_event_hash(event);
  printf("new 3\n");
  // FlKeyboardPendingEvent* self2 = FL_KEYBOARD_PENDING_EVENT(self);
  printf("new 4\n");
  return self2;
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

  // An array of #FlKeyboardPendingEvent. FlKeyboardManager must manually
  // release the elements unless it is transferring them to
  // pending_redispatches.
  GPtrArray* pending_responds;
  // An array of #FlKeyboardPendingEvent. FlKeyboardManager must manually
  // release the elements.
  GPtrArray* pending_redispatches;

  uint64_t last_sequence_id;
};

G_DEFINE_TYPE(FlKeyboardManager, fl_keyboard_manager, G_TYPE_OBJECT);

// Disposes of an FlKeyboardManager instance.
static void fl_keyboard_manager_dispose(GObject* object) {
  FlKeyboardManager* self = FL_KEYBOARD_MANAGER(object);

  if (self->text_input_plugin != nullptr)
    g_clear_object(&self->text_input_plugin);
  g_ptr_array_free(self->responder_list, TRUE);
  g_ptr_array_set_free_func(self->pending_responds, g_object_unref);
  g_ptr_array_free(self->pending_responds, TRUE);
  g_ptr_array_set_free_func(self->pending_redispatches, g_object_unref);
  g_ptr_array_free(self->pending_redispatches, TRUE);

  G_OBJECT_CLASS(fl_keyboard_manager_parent_class)->dispose(object);
}

// Initializes the FlKeyboardManager class methods.
static void fl_keyboard_manager_class_init(FlKeyboardManagerClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_manager_dispose;
}

static void fl_keyboard_manager_init(FlKeyboardManager* self) {}

// Compare a #FlKeyboardPendingEvent with the given sequence_id. The #needle
// should be a pointer to uint64_t sequence_id.
static gboolean compare_pending_by_sequence_id(
    gconstpointer pending,
    gconstpointer needle_sequence_id) {
  uint64_t sequence_id = *reinterpret_cast<const uint64_t*>(needle_sequence_id);
  return static_cast<const FlKeyboardPendingEvent*>(pending)->sequence_id ==
         sequence_id;
}

// Compare a #FlKeyboardPendingEvent with the given hash. The #needle should be
// a pointer to uint64_t hash.
static gboolean compare_pending_by_hash(gconstpointer pending,
                                        gconstpointer needle_hash) {
  uint64_t hash = *reinterpret_cast<const uint64_t*>(needle_hash);
  return static_cast<const FlKeyboardPendingEvent*>(pending)->hash == hash;
}

static bool fl_keyboard_manager_remove_redispatched(FlKeyboardManager* self,
                                                    uint64_t hash) {
  guint result_index;
  gboolean found = g_ptr_array_find_with_equal_func(
      self->pending_redispatches, static_cast<const uint64_t*>(&hash),
      compare_pending_by_hash, &result_index);
  if (found) {
    gpointer removed =
        g_ptr_array_remove_index_fast(self->pending_redispatches, result_index);
    g_return_val_if_fail(removed != nullptr, TRUE);
    g_object_unref(removed);
    return TRUE;
  } else {
    return FALSE;
  }
}

static void responder_handle_event_callback(bool handled,
                                            gpointer user_data_ptr) {
  g_return_if_fail(FL_IS_KEYBOARD_MANAGER_USER_DATA(user_data_ptr));
  FlKeyboardManagerUserData* user_data =
      FL_KEYBOARD_MANAGER_USER_DATA(user_data_ptr);
  printf("callback 2\n");
  FlKeyboardManager* self = user_data->manager;

  guint result_index = -1;
  gboolean found = g_ptr_array_find_with_equal_func(
      self->pending_responds, &user_data->sequence_id,
      compare_pending_by_sequence_id, &result_index);
  g_return_if_fail(found);
  printf("callback 3\n");
  FlKeyboardPendingEvent* pending = FL_KEYBOARD_PENDING_EVENT(
      g_ptr_array_index(self->pending_responds, result_index));
  printf("callback 4 unrep %zu\n", pending->unreplied);
  g_return_if_fail(pending != nullptr);
  g_return_if_fail(pending->unreplied > 0);
  pending->unreplied -= 1;
  pending->any_handled = pending->any_handled || handled;
  // All responders have replied.
  if (pending->unreplied == 0) {
    printf("callback 5\n");
    g_object_unref(user_data_ptr);
    g_ptr_array_remove_index_fast(self->pending_responds, result_index);
    bool should_redispatch = false;
    if (!pending->any_handled) {
      printf("callback 6\n");
      // If no responders have handled, send it to text plugin.
      if (self->text_input_plugin == nullptr ||
          !fl_text_input_plugin_filter_keypress(self->text_input_plugin,
                                                pending->event)) {
        // If text plugin doesn't handle either, redispatch.
        should_redispatch = true;
      }
    }
    if (should_redispatch) {
      printf("callback 7\n");
      g_ptr_array_add(self->pending_redispatches, pending);
      self->redispatch_callback(reinterpret_cast<GdkEvent*>(pending->event));
    } else {
      printf("callback 8\n");
      g_object_unref(pending);
    }
  }
  printf("callback ret\n");
}

FlKeyboardManager* fl_keyboard_manager_new(
    FlTextInputPlugin* text_input_plugin,
    FlKeyboardManagerRedispatcher redispatch_callback) {
  g_return_val_if_fail(text_input_plugin == nullptr ||
                           FL_IS_TEXT_INPUT_PLUGIN(text_input_plugin),
                       nullptr);
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

void fl_keyboard_manager_add_responder(FlKeyboardManager* self,
                                       FlKeyResponder* responder) {
  g_return_if_fail(FL_IS_KEYBOARD_MANAGER(self));
  g_return_if_fail(responder != nullptr);

  g_ptr_array_add(self->responder_list, responder);
  printf("after add len %u\n", self->responder_list->len);
}

static void dispatch_pending_to_responder(gpointer responder_data,
                                          gpointer foreach_data_ptr) {
  DispatchPendingToResponderForeachData* foreach_data =
      reinterpret_cast<DispatchPendingToResponderForeachData*>(
          foreach_data_ptr);
  FlKeyResponder* responder = FL_KEY_RESPONDER(responder_data);
  fl_key_responder_handle_event(responder, foreach_data->event,
                                responder_handle_event_callback,
                                foreach_data->user_data);
}

gboolean fl_keyboard_manager_handle_event(FlKeyboardManager* self,
                                          GdkEventKey* event) {
  g_return_val_if_fail(FL_IS_KEYBOARD_MANAGER(self), FALSE);
  g_return_val_if_fail(event != nullptr, FALSE);

  uint64_t incoming_hash = fl_keyboard_manager_get_event_hash(event);
  printf("Handle 1\n");
  if (fl_keyboard_manager_remove_redispatched(self, incoming_hash)) {
    printf("Handle ret\n");
    return FALSE;
  }

  printf("Handle 2 len %u\n", self->responder_list->len);
  FlKeyboardPendingEvent* pending = fl_keyboard_pending_event_new(
      event, ++self->last_sequence_id, self->responder_list->len);

  printf("Handle 2.5\n");
  g_ptr_array_add(self->pending_responds, pending);
  printf("Handle 3\n");
  FlKeyboardManagerUserData* user_data =
      fl_keyboard_manager_user_data_new(self, pending->sequence_id);
  DispatchPendingToResponderForeachData data{
      .event = event,
      .user_data = user_data,
  };
  g_ptr_array_foreach(self->responder_list, dispatch_pending_to_responder,
                      &data);
  printf("Handle 4\n");

  return TRUE;
}

gboolean fl_keyboard_manager_state_clear(FlKeyboardManager* self) {
  g_return_val_if_fail(FL_IS_KEYBOARD_MANAGER(self), FALSE);
  return self->pending_responds->len == 0 &&
         self->pending_redispatches->len == 0;
}
