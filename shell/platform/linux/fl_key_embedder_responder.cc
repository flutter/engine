// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_key_embedder_responder.h"

#include <gtk/gtk.h>
#include <cinttypes>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/fl_key_embedder_responder_private.h"
#include "flutter/shell/platform/linux/key_mapping.h"

// The code prefix for unrecognized keys that are unique to Gtk, generated from
// platform-specific codes.
constexpr uint64_t kGtkKeyIdPlane = 0x00600000000;

constexpr uint64_t kMicrosecondsPerMillisecond = 1000;

static uint64_t lookup_hash_table(GHashTable* table, uint64_t key) {
  return gpointer_to_uint64(
      g_hash_table_lookup(table, uint64_to_gpointer(key)));
}

static uint64_t event_to_physical_key(const GdkEventKey* event,
                                      GHashTable* table) {
  uint64_t record = lookup_hash_table(table, event->hardware_keycode);
  if (record != 0) {
    return record;
  }
  return kGtkKeyIdPlane | event->hardware_keycode;
}

static uint64_t to_lower(uint64_t n) {
  constexpr uint64_t lower_a = 0x61;
  constexpr uint64_t upper_a = 0x41;
  constexpr uint64_t upper_z = 0x5a;

  constexpr uint64_t lower_a_grave = 0xe0;
  constexpr uint64_t upper_a_grave = 0xc0;
  constexpr uint64_t upper_thorn = 0xde;
  constexpr uint64_t division = 0xf7;

  // ASCII range.
  if (n >= upper_a && n <= upper_z) {
    return n - upper_a + lower_a;
  }

  // EASCII range.
  if (n >= upper_a_grave && n <= upper_thorn && n != division) {
    return n - upper_a_grave + lower_a_grave;
  }

  return n;
}

/* Define FlKeyEmbedderUserData */

/**
 * FlKeyEmbedderUserData:
 * The user_data used when #FlKeyEmbedderResponder sends message through the
 * embedder.SendKeyEvent API.
 */
#define FL_TYPE_EMBEDDER_USER_DATA fl_key_embedder_user_data_get_type()
G_DECLARE_FINAL_TYPE(FlKeyEmbedderUserData,
                     fl_key_embedder_user_data,
                     FL,
                     KEY_EMBEDDER_USER_DATA,
                     GObject);

struct _FlKeyEmbedderUserData {
  GObject parent_instance;

  FlKeyEmbedderResponder* responder;
  FlKeyResponderAsyncCallback callback;
  gpointer user_data;
};

G_DEFINE_TYPE(FlKeyEmbedderUserData, fl_key_embedder_user_data, G_TYPE_OBJECT)

static void fl_key_embedder_user_data_dispose(GObject* object);

static void fl_key_embedder_user_data_class_init(
    FlKeyEmbedderUserDataClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_key_embedder_user_data_dispose;
}

static void fl_key_embedder_user_data_init(FlKeyEmbedderUserData* self) {}

static void fl_key_embedder_user_data_dispose(GObject* object) {
  g_return_if_fail(FL_IS_KEY_EMBEDDER_USER_DATA(object));
  FlKeyEmbedderUserData* self = FL_KEY_EMBEDDER_USER_DATA(object);
  if (self->responder != nullptr) {
    g_object_remove_weak_pointer(
        G_OBJECT(self->responder),
        reinterpret_cast<gpointer*>(&(self->responder)));
    self->responder = nullptr;
  }
}

// Creates a new FlKeyChannelUserData private class with all information.
//
// The callback and the user_data might be nullptr.
static FlKeyEmbedderUserData* fl_key_embedder_user_data_new(
    FlKeyEmbedderResponder* responder,
    FlKeyResponderAsyncCallback callback,
    gpointer user_data) {
  FlKeyEmbedderUserData* self = FL_KEY_EMBEDDER_USER_DATA(
      g_object_new(FL_TYPE_EMBEDDER_USER_DATA, nullptr));

  self->responder = responder;
  // Add a weak pointer so we can know if the key event responder disappeared
  // while the framework was responding.
  g_object_add_weak_pointer(G_OBJECT(responder),
                            reinterpret_cast<gpointer*>(&(self->responder)));
  self->callback = callback;
  self->user_data = user_data;
  return self;
}

/* Define FlKeyEmbedderResponder */

namespace {

typedef enum {
  kStateLogicUndecided,
  kStateLogicNormal,
  kStateLogicReversed,
} StateLogicInferrence;

}

struct _FlKeyEmbedderResponder {
  GObject parent_instance;

  // A weak pointer to the engine the responder is attached to.
  FlEngine* engine;

  // Internal record for states of whether a key is pressed.
  //
  // It is a map from Flutter physical key to Flutter logical key.  Both keys
  // and values are directly stored uint64s.  This table is freed by the
  // responder.
  GHashTable* pressing_records;

  // Internal record for states of whether a lock mode is enabled.
  //
  // It is a bit mask composed of GTK mode bits.
  guint lock_records;

  // The inferred mode indicating whether the CapsLock state logic is reversed
  // on this platform.
  //
  // For more information, see #update_caps_lock_state_logic_inferrence.
  StateLogicInferrence caps_lock_state_logic_inferrence;

  // A static map from XKB code to Flutter's physical key code.
  //
  // Both keys and values are directly stored uint64s.  This table is freed by
  // the responder.
  GHashTable* xkb_to_physical_key;

  // A static map from GTK keyval to Flutter's logical key code
  //
  // Both keys and values are directly stored uint64s.  This table is freed by
  // the responder.
  GHashTable* keyval_to_logical_key;

  // A static map from GTK modifier bits to #FlKeyEmbedderCheckedKey to
  // configure the modifier keys that needs to be tracked and kept synchronous
  // on.
  //
  // The keys are directly stored guints.  The values must be freed with g_free.
  // This table is freed by the responder.
  GHashTable* modifier_bit_to_checked_keys;

  // A static map from GTK modifier bits to #FlKeyEmbedderCheckedKey to
  // configure the lock mode bits that needs to be tracked and kept synchronous
  // on.
  //
  // The keys are directly stored guints.  The values must be freed with g_free.
  // This table is freed by the responder.
  GHashTable* lock_bit_to_checked_keys;

  // A static map generated by reverse mapping lock_bit_to_checked_keys.
  //
  // It is a map from primary physical keys to lock bits.  Both keys and values
  // are directly stored uint64s.  This table is freed by the responder.
  GHashTable* physical_key_to_lock_bit;
};

static void fl_key_embedder_responder_iface_init(
    FlKeyResponderInterface* iface);
static void fl_key_embedder_responder_dispose(GObject* object);

#define FL_TYPE_EMBEDDER_RESPONDER_USER_DATA \
  fl_key_embedder_responder_get_type()
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

// Initializes the FlKeyEmbedderResponder class methods.
static void fl_key_embedder_responder_class_init(
    FlKeyEmbedderResponderClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_key_embedder_responder_dispose;
}

// Initializes an FlKeyEmbedderResponder instance.
static void fl_key_embedder_responder_init(FlKeyEmbedderResponder* self) {}

// Disposes of an FlKeyEmbedderResponder instance.
static void fl_key_embedder_responder_dispose(GObject* object) {
  FlKeyEmbedderResponder* self = FL_KEY_EMBEDDER_RESPONDER(object);

  g_clear_pointer(&self->pressing_records, g_hash_table_unref);
  g_clear_pointer(&self->xkb_to_physical_key, g_hash_table_unref);
  g_clear_pointer(&self->keyval_to_logical_key, g_hash_table_unref);
  g_clear_pointer(&self->modifier_bit_to_checked_keys, g_hash_table_unref);
  g_clear_pointer(&self->lock_bit_to_checked_keys, g_hash_table_unref);

  G_OBJECT_CLASS(fl_key_embedder_responder_parent_class)->dispose(object);
}

static void initialize_physical_key_to_lock_bit_loop_body(gpointer lock_bit,
                                                          gpointer value,
                                                          gpointer user_data) {
  FlKeyEmbedderCheckedKey* checked_key =
      reinterpret_cast<FlKeyEmbedderCheckedKey*>(value);
  GHashTable* table = reinterpret_cast<GHashTable*>(user_data);
  g_hash_table_insert(table,
                      uint64_to_gpointer(checked_key->primary_physical_key),
                      GUINT_TO_POINTER(lock_bit));
}

// Creates a new FlKeyEmbedderResponder instance, with a messenger used to send
// messages to the framework, an FlTextInputPlugin used to handle key events
// that the framework doesn't handle. Mainly for testing purposes, it also takes
// an optional callback to call when a response is received, and an optional
// embedder name to use when sending messages.
FlKeyEmbedderResponder* fl_key_embedder_responder_new(FlEngine* engine) {
  g_return_val_if_fail(FL_IS_ENGINE(engine), nullptr);

  FlKeyEmbedderResponder* self = FL_KEY_EMBEDDER_RESPONDER(
      g_object_new(FL_TYPE_EMBEDDER_RESPONDER_USER_DATA, nullptr));

  self->engine = engine;
  // Add a weak pointer so we can know if the key event responder disappeared
  // while the framework was responding.
  g_object_add_weak_pointer(G_OBJECT(engine),
                            reinterpret_cast<gpointer*>(&(self->engine)));

  self->pressing_records = g_hash_table_new(g_direct_hash, g_direct_equal);
  self->lock_records = 0;
  self->caps_lock_state_logic_inferrence = kStateLogicUndecided;

  self->xkb_to_physical_key = g_hash_table_new(g_direct_hash, g_direct_equal);
  initialize_xkb_to_physical_key(self->xkb_to_physical_key);

  self->keyval_to_logical_key = g_hash_table_new(g_direct_hash, g_direct_equal);
  initialize_gtk_keyval_to_logical_key(self->keyval_to_logical_key);

  self->modifier_bit_to_checked_keys =
      g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
  initialize_modifier_bit_to_checked_keys(self->modifier_bit_to_checked_keys);

  self->lock_bit_to_checked_keys =
      g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);
  initialize_lock_bit_to_checked_keys(self->lock_bit_to_checked_keys);

  self->physical_key_to_lock_bit =
      g_hash_table_new(g_direct_hash, g_direct_equal);
  g_hash_table_foreach(self->lock_bit_to_checked_keys,
                       initialize_physical_key_to_lock_bit_loop_body,
                       self->physical_key_to_lock_bit);

  return self;
}

/* Implement FlKeyEmbedderUserData */

static uint64_t event_to_logical_key(const GdkEventKey* event,
                                     GHashTable* table) {
  guint keyval = event->keyval;
  uint64_t record = lookup_hash_table(table, keyval);
  if (record != 0) {
    return record;
  }
  // EASCII range
  if (keyval < 256) {
    return to_lower(keyval);
  }
  // Auto-generate key
  return kGtkKeyIdPlane | keyval;
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

// Handles a response from the embedder API to a key event sent to the framework
// earlier.
static void handle_response(bool handled, gpointer user_data) {
  g_autoptr(FlKeyEmbedderUserData) data = FL_KEY_EMBEDDER_USER_DATA(user_data);

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

// Sends a synthesized event to the engine with no demand for callback.
static void synthesize_simple_event(FlKeyEmbedderResponder* self,
                                    FlutterKeyEventType type,
                                    uint64_t physical,
                                    uint64_t logical,
                                    double timestamp) {
  FlutterKeyEvent out_event;
  out_event.struct_size = sizeof(out_event);
  out_event.timestamp = timestamp;
  out_event.type = type;
  out_event.physical = physical;
  out_event.logical = logical;
  out_event.character = nullptr;
  out_event.synthesized = true;
  fl_engine_send_key_event(self->engine, &out_event, nullptr, nullptr);
}

namespace {

// Context variables for the foreach call used to synchronize pressing states
// and lock states.
typedef struct {
  FlKeyEmbedderResponder* self;
  guint state;
  uint64_t event_physical_key;
  bool is_down;
  double timestamp;
} SyncStateLoopContext;

}  // namespace

// The loop body to synchronize pressing states.
static void synchronize_pressed_states_loop_body(gpointer key,
                                                 gpointer value,
                                                 gpointer user_data) {
  SyncStateLoopContext* context =
      reinterpret_cast<SyncStateLoopContext*>(user_data);
  FlKeyEmbedderCheckedKey* checked_key =
      reinterpret_cast<FlKeyEmbedderCheckedKey*>(value);

  const guint modifier_bit = GPOINTER_TO_INT(key);
  FlKeyEmbedderResponder* self = context->self;
  const uint64_t physical_keys[] = {
      checked_key->primary_physical_key,
      checked_key->secondary_physical_key,
  };
  const guint length = checked_key->secondary_physical_key == 0 ? 1 : 2;

  const bool pressed_by_state = (context->state & modifier_bit) != 0;
  bool pressed_by_record = false;

  for (guint physical_key_idx = 0; physical_key_idx < length;
       physical_key_idx++) {
    const uint64_t physical_key = physical_keys[physical_key_idx];
    const uint64_t pressed_logical_key =
        lookup_hash_table(self->pressing_records, physical_key);
    // If this event is an event of the key, then the state will be flipped but
    // haven't reflected on the state. Flip it manually here.
    const bool adjusted_this_key_pressed =
        (pressed_logical_key != 0) ^
        (physical_key == context->event_physical_key);

    if (adjusted_this_key_pressed) {
      pressed_by_record = true;
      if (!pressed_by_state) {
        g_return_if_fail(physical_key != context->event_physical_key);
        synthesize_simple_event(self, kFlutterKeyEventTypeUp, physical_key,
                                pressed_logical_key, context->timestamp);
        g_hash_table_remove(self->pressing_records,
                            uint64_to_gpointer(physical_key));
      }
    }
  }
  if (pressed_by_state && !pressed_by_record) {
    uint64_t physical_key = checked_key->primary_physical_key;
    g_return_if_fail(physical_key != context->event_physical_key);
    uint64_t logical_key = checked_key->primary_logical_key;
    synthesize_simple_event(self, kFlutterKeyEventTypeDown, physical_key,
                            logical_key, context->timestamp);
    g_hash_table_insert(self->pressing_records,
                        uint64_to_gpointer(physical_key),
                        uint64_to_gpointer(logical_key));
  }
}

// Update the pressing record.
//
// If `logical_key` is 0, the record will be set as "released".  Otherwise, the
// record will be set as "pressed" with this logical key.  This function asserts
// that the key is pressed if the caller asked to release, and vice versa.
static void update_pressing_state(FlKeyEmbedderResponder* self,
                                  uint64_t physical_key,
                                  uint64_t logical_key) {
  if (logical_key != 0) {
    g_return_if_fail(lookup_hash_table(self->pressing_records, physical_key) ==
                     0);
    g_hash_table_insert(self->pressing_records,
                        uint64_to_gpointer(physical_key),
                        uint64_to_gpointer(logical_key));
  } else {
    g_return_if_fail(lookup_hash_table(self->pressing_records, physical_key) !=
                     0);
    g_hash_table_remove(self->pressing_records,
                        uint64_to_gpointer(physical_key));
  }
}

// Update the lock record.
//
// If `is_down` is false, this function is a no-op.  Otherwise, this function
// finds the lock bit corresponding to `physical_key`, and flips its bit.
static void possibly_update_lock_bit(FlKeyEmbedderResponder* self,
                                     uint64_t physical_key,
                                     bool is_down) {
  if (!is_down) {
    return;
  }
  const guint mode_bit = GPOINTER_TO_UINT(g_hash_table_lookup(
      self->physical_key_to_lock_bit, uint64_to_gpointer(physical_key)));
  if (mode_bit != 0) {
    self->lock_records ^= mode_bit;
  }
}

// Find the stage # by the current record, which should be the recorded stage
// before the event.
static int find_stage_by_record(bool is_down, bool is_enabled) {
  constexpr int stage_by_record_index[] = {
      0,  // is_down: 0,  is_enabled: 0
      2,  //          0               1
      3,  //          1               0
      1   //          1               1
  };
  return stage_by_record_index[(is_down << 1) + is_enabled];
}

// Find the stage # by an event for the target key, which should be inferred
// stage before the event.
static int find_stage_by_self_event(int stage_by_record,
                                    bool is_down_event,
                                    bool is_state_on,
                                    bool reverse_state_logic) {
  if (!is_state_on) {
    return reverse_state_logic ? 2 : 0;
  }
  if (is_down_event) {
    return reverse_state_logic ? 0 : 2;
  }
  return stage_by_record;
}

// Find the stage # by an event for a non-target key, which should be inferred
// stage during the event.
static int find_stage_by_others_event(int stage_by_record, bool is_state_on) {
  g_return_val_if_fail(stage_by_record >= 0 && stage_by_record < 4,
                       stage_by_record);
  if (!is_state_on) {
    return 0;
  }
  if (stage_by_record == 0) {
    return 1;
  }
  return stage_by_record;
}

// Infer caps_lock_state_logic_inferrence if applicable.
//
// In most cases, when a lock key is pressed or released, its event has the
// key's state as 0-1-1-1 for the 4 stages (as documented in
// #synchronize_lock_states_loop_body) respectively.  But in very rare cases it
// produces 1-1-0-1, which we call "reversed state logic".  This is observed
// when using Chrome Remote Desktop on macOS.
//
// To detect whether the current platform is normal or reversed, this function
// is called on the first down event of CapsLock before calculating stages.
// This function then store the inferred mode as
// self->caps_lock_state_logic_inferrence.
//
// Note that this does not help if the same app session is used alternatively
// between a reversed platform and a normal platform.  But this is the best we
// can do.
static void update_caps_lock_state_logic_inferrence(
    FlKeyEmbedderResponder* self,
    bool is_down_event,
    bool enabled_by_state,
    int stage_by_record) {
  if (self->caps_lock_state_logic_inferrence != kStateLogicUndecided) {
    return;
  }
  if (!is_down_event) {
    return;
  }
  const int stage_by_event = find_stage_by_self_event(
      stage_by_record, is_down_event, enabled_by_state, false);
  if ((stage_by_event == 0 && stage_by_record == 2) ||
      (stage_by_event == 2 && stage_by_record == 0)) {
    self->caps_lock_state_logic_inferrence = kStateLogicReversed;
  } else {
    self->caps_lock_state_logic_inferrence = kStateLogicNormal;
  }
}

// The loop body to synchronize lock states.
//
// This function might modify self->caps_lock_state_logic_inferrence.
static void synchronize_lock_states_loop_body(gpointer key,
                                              gpointer value,
                                              gpointer user_data) {
  SyncStateLoopContext* context =
      reinterpret_cast<SyncStateLoopContext*>(user_data);
  FlKeyEmbedderCheckedKey* checked_key =
      reinterpret_cast<FlKeyEmbedderCheckedKey*>(value);

  guint modifier_bit = GPOINTER_TO_INT(key);
  FlKeyEmbedderResponder* self = context->self;

  const uint64_t logical_key = checked_key->primary_logical_key;
  const uint64_t physical_key = checked_key->primary_physical_key;

  // A lock mode key can be at any of a 4-stage cycle. The following table lists
  // the definition of each stage (TruePressed and TrueEnabled), the event of
  // the lock key between every 2 stages (SelfType and SelfState), and the event
  // of other keys at each stage (OthersState). Notice that on certain platforms
  // SelfState uses a reversed rule for certain keys (SelfState(rvsd)).
  //
  //               #    [0]         [1]          [2]           [3]
  //     TruePressed: Released    Pressed      Released      Pressed
  //     TrueEnabled: Disabled    Enabled      Enabled       Disabled
  //        SelfType:         Down         Up           Down            Up
  //       SelfState:          0           1             1              1
  // SelfState(rvsd):          1           1             0              1
  //     OthersState:    0           1            1              1
  //
  // Except for stage 0, we can't derive the exact stage just from event
  // information. Choose the stage that requires the minimal synthesization.

  const uint64_t pressed_logical_key =
      lookup_hash_table(self->pressing_records, physical_key);
  // For simplicity, we're not considering remapped lock keys until we meet such
  // cases.
  g_return_if_fail(pressed_logical_key == 0 ||
                   pressed_logical_key == logical_key);
  const int stage_by_record = find_stage_by_record(
      pressed_logical_key != 0, (self->lock_records & modifier_bit) != 0);

  const bool enabled_by_state = (context->state & modifier_bit) != 0;
  const bool is_self_event = physical_key == context->event_physical_key;
  if (is_self_event && checked_key->is_caps_lock) {
    update_caps_lock_state_logic_inferrence(self, context->is_down,
                                            enabled_by_state, stage_by_record);
    g_return_if_fail(self->caps_lock_state_logic_inferrence !=
                     kStateLogicUndecided);
  }
  const bool reverse_state_logic =
      checked_key->is_caps_lock &&
      self->caps_lock_state_logic_inferrence == kStateLogicReversed;
  const int stage_by_event =
      is_self_event
          ? find_stage_by_self_event(stage_by_record, context->is_down,
                                     enabled_by_state, reverse_state_logic)
          : find_stage_by_others_event(stage_by_record, enabled_by_state);

  // The destination stage is equal to stage_by_event but shifted cyclically to
  // be no less than stage_by_record.
  constexpr int kNumStages = 4;
  const int destination_stage = stage_by_event >= stage_by_record
                                    ? stage_by_event
                                    : stage_by_event + kNumStages;

  g_return_if_fail(stage_by_record <= destination_stage);
  if (stage_by_record == destination_stage) {
    return;
  }
  for (int current_stage = stage_by_record; current_stage < destination_stage;
       current_stage += 1) {
    if (current_stage == 9) {
      return;
    }

    const int standard_current_stage =
        current_stage >= 4 ? current_stage - 4 : current_stage;
    const bool is_down_event =
        standard_current_stage == 0 || standard_current_stage == 2;
    FlutterKeyEventType type =
        is_down_event ? kFlutterKeyEventTypeDown : kFlutterKeyEventTypeUp;
    update_pressing_state(self, physical_key, is_down_event ? logical_key : 0);
    possibly_update_lock_bit(self, physical_key, is_down_event);
    synthesize_simple_event(self, type, physical_key, logical_key,
                            context->timestamp);
  }
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

  const uint64_t physical_key =
      event_to_physical_key(event, self->xkb_to_physical_key);
  const uint64_t logical_key =
      event_to_logical_key(event, self->keyval_to_logical_key);
  const double timestamp = event_to_timestamp(event);
  const bool is_down_event = event->type == GDK_KEY_PRESS;

  SyncStateLoopContext sync_pressed_state_context;
  sync_pressed_state_context.self = self;
  sync_pressed_state_context.state = event->state;
  sync_pressed_state_context.timestamp = timestamp;
  sync_pressed_state_context.is_down = is_down_event;
  sync_pressed_state_context.event_physical_key = physical_key;

  // Update lock mode states
  g_hash_table_foreach(self->lock_bit_to_checked_keys,
                       synchronize_lock_states_loop_body,
                       &sync_pressed_state_context);

  // Construct the real event
  const uint64_t last_logical_record =
      lookup_hash_table(self->pressing_records, physical_key);

  FlutterKeyEvent out_event;
  out_event.struct_size = sizeof(out_event);
  out_event.timestamp = timestamp;
  out_event.physical = physical_key;
  out_event.logical = logical_key;
  out_event.character = nullptr;
  out_event.synthesized = false;

  g_autofree char* character_to_free = nullptr;
  if (is_down_event) {
    if (last_logical_record) {
      // A key has been pressed that has the exact physical key as a currently
      // pressed one, usually indicating multiple keyboards are pressing keys
      // with the same physical key, or the up event was lost during a loss of
      // focus. The down event is ignored.
      callback(true, user_data);
      return;
    } else {
      out_event.type = kFlutterKeyEventTypeDown;
      character_to_free = event_to_character(event);  // Might be null
      out_event.character = character_to_free;
    }
  } else {  // is_down_event false
    if (!last_logical_record) {
      // The physical key has been released before. It might indicate a missed
      // event due to loss of focus, or multiple keyboards pressed keys with the
      // same physical key. Ignore the up event.
      callback(true, user_data);
      return;
    } else {
      out_event.type = kFlutterKeyEventTypeUp;
    }
  }
  update_pressing_state(self, physical_key, is_down_event ? logical_key : 0);
  possibly_update_lock_bit(self, physical_key, is_down_event);

  // Update pressing states
  g_hash_table_foreach(self->modifier_bit_to_checked_keys,
                       synchronize_pressed_states_loop_body,
                       &sync_pressed_state_context);

  FlKeyEmbedderUserData* response_data =
      fl_key_embedder_user_data_new(self, callback, user_data);

  fl_engine_send_key_event(self->engine, &out_event, handle_response,
                           response_data);
}
