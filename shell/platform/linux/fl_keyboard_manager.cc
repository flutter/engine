// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_keyboard_manager.h"

#include <array>
#include <cinttypes>
#include <memory>
#include <set>
#include <string>
#include <X11/Xutil.h>

#include "flutter/shell/platform/linux/fl_key_channel_responder.h"
#include "flutter/shell/platform/linux/fl_key_embedder_responder.h"
#include "flutter/shell/platform/linux/key_mapping.h"

// Turn on this flag to print complete layout data when switching IMEs. The data
// is used in unit tests.
#define DEBUG_PRINT_LAYOUT

/* Declarations of private classes */

G_DECLARE_FINAL_TYPE(FlKeyboardPendingEvent,
                     fl_keyboard_pending_event,
                     FL,
                     KEYBOARD_PENDING_EVENT,
                     GObject);

#define FL_TYPE_KEYBOARD_MANAGER_USER_DATA \
  fl_keyboard_manager_user_data_get_type()
G_DECLARE_FINAL_TYPE(FlKeyboardManagerUserData,
                     fl_keyboard_manager_user_data,
                     FL,
                     KEYBOARD_MANAGER_USER_DATA,
                     GObject);

/* End declarations */

namespace {

using flutter::GroupLayout;
using flutter::GroupLayouts;
using flutter::kLayoutSize;

// Context variables for the foreach call used to dispatch events to responders.
typedef struct {
  FlKeyEvent* event;
  FlKeyboardManagerUserData* user_data;
} DispatchToResponderLoopContext;

bool is_eascii(uint16_t character) {
  return character < 256;
}

#ifdef DEBUG_PRINT_LAYOUT
// Prints layout entries that will be parsed by `MockLayoutData`.
void debug_format_layout_data(std::string& debug_layout_data,
                                uint16_t keycode,
                                uint16_t clue1,
                                uint16_t clue2) {
  constexpr int kBufferSize = 30;

  char buffer[kBufferSize];
  buffer[0] = 0;
  buffer[kBufferSize - 1] = 0;
  if (keycode % 4 == 0) {
    snprintf(buffer, kBufferSize, "    /* 0x%02x */ ", keycode);
  }
  debug_layout_data.append(buffer);

  snprintf(buffer, kBufferSize, "0x%04x, 0x%04x,%s", clue1, clue2,
           (keycode % 4 == 3) ? "\n" : " ");
  debug_layout_data.append(buffer);
}

// typedef std::vector<uint16_t> PrintLayoutRowKeys;
// typedef std::pair<int, PrintLayoutRowKeys> PrintLayoutRow;
// void debug_print_layout_result(const flutter::GroupLayout& layout) {
//   std::vector<uint16_t> row1 = {
//       // `    1     2     3     4     5     6
//       0x31, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
//       // 7    8     9     0     -     =
//       0x10, 0x11, 0x12, 0x13, 0x14, 0x15};
//   std::vector<uint16_t> row2 = {
//       // Q    W     E     R     T     Y     U
//       0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e,
//       // I    O     P     [     ]    '\'
//       0x1f, 0x20, 0x21, 0x22, 0x23, 0x33};
//   std::vector<uint16_t> row3 = {
//       // A    S     D     F     G     H     J     K     L     ;     '
//       0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f, 0x30};
//   std::vector<uint16_t> row4 = {
//       // ⍉    Z     X     C     V     B     N     M     ,     .     /
//       0x5e, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d,
//   };
//   constexpr const char* kIntlBackslashChar = "⍉";
//   constexpr const char* kNotMappedChar = "◌";
//   constexpr uint64_t kLogicalIntlBackslash = 0x200000020;

//   std::vector<PrintLayoutRow> rows = {
//       std::make_pair(0, row1),
//       std::make_pair(3, row2),
//       std::make_pair(4, row3),
//       std::make_pair(3, row4),
//   };
//   for (auto row : rows) {
//     for (int space_count = 0; space_count < row.first; space_count += 1) {
//       printf("  ");
//     }
//     for (uint16_t keycode : row.second) {
//       uint64_t mapped = layout[keycode];
//       switch (mapped) {
//         case 0:
//           printf("[%s] ", kNotMappedChar);
//           break;
//         case kLogicalIntlBackslash:
//           printf("[%s] ", kIntlBackslashChar);
//           break;
//         default:
//           if (mapped < 256) {
//             printf("[%c] ", static_cast<char>(mapped));
//           } else {
//             printf("0x%lx ", mapped);
//           }
//       }
//     }
//     printf("\n");
//   }
// }
#endif

}  // namespace

uint64_t flutter::get_logical_key_from_layout(const FlKeyEvent* event, const GroupLayouts* group_layouts) {
  guint8 group = event->group;
  guint16 keycode = event->keycode;
  if (keycode >= kLayoutSize) {
    return 0;
  }

  if (group_layouts != nullptr) {
    auto found_group_layout = group_layouts->find(group);
    if (found_group_layout != group_layouts->end()) {
      return found_group_layout->second[keycode];
    }
  }
  return 0;
}

/* Define FlKeyboardPendingEvent */

/**
 * FlKeyboardPendingEvent:
 * A record for events that have been received by the manager, but
 * dispatched to other objects, whose results have yet to return.
 *
 * This object is used by both the "pending_responds" list and the
 * "pending_redispatches" list.
 */

struct _FlKeyboardPendingEvent {
  GObject parent_instance;

  // The target event.
  //
  // This is freed by #FlKeyboardPendingEvent if not null.
  std::unique_ptr<FlKeyEvent> event;

  // Self-incrementing ID attached to an event sent to the framework.
  //
  // Used to identify pending responds.
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

static void fl_keyboard_pending_event_dispose(GObject* object) {
  // Redundant, but added so that we don't get a warning about unused function
  // for FL_IS_KEYBOARD_PENDING_EVENT.
  g_return_if_fail(FL_IS_KEYBOARD_PENDING_EVENT(object));

  FlKeyboardPendingEvent* self = FL_KEYBOARD_PENDING_EVENT(object);
  if (self->event != nullptr) {
    fl_key_event_dispose(self->event.release());
  }
  G_OBJECT_CLASS(fl_keyboard_pending_event_parent_class)->dispose(object);
}

static void fl_keyboard_pending_event_class_init(
    FlKeyboardPendingEventClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_pending_event_dispose;
}

static void fl_keyboard_pending_event_init(FlKeyboardPendingEvent* self) {}

// Calculates a unique ID for a given FlKeyEvent object to use for
// identification of responses from the framework.
static uint64_t fl_keyboard_manager_get_event_hash(FlKeyEvent* event) {
  // Combine the event timestamp, the type of event, and the hardware keycode
  // (scan code) of the event to come up with a unique id for this event that
  // can be derived solely from the event data itself, so that we can identify
  // whether or not we have seen this event already.
  guint64 type =
      static_cast<uint64_t>(event->is_press ? GDK_KEY_PRESS : GDK_KEY_RELEASE);
  guint64 keycode = static_cast<uint64_t>(event->keycode);
  return (event->time & 0xffffffff) | ((type & 0xffff) << 32) |
         ((keycode & 0xffff) << 48);
}

// Create a new FlKeyboardPendingEvent by providing the target event,
// the sequence ID, and the number of responders that will reply.
//
// This will acquire the ownership of the event.
static FlKeyboardPendingEvent* fl_keyboard_pending_event_new(
    std::unique_ptr<FlKeyEvent> event,
    uint64_t sequence_id,
    size_t to_reply) {
  FlKeyboardPendingEvent* self = FL_KEYBOARD_PENDING_EVENT(
      g_object_new(fl_keyboard_pending_event_get_type(), nullptr));

  self->event = std::move(event);
  self->sequence_id = sequence_id;
  self->unreplied = to_reply;
  self->any_handled = false;
  self->hash = fl_keyboard_manager_get_event_hash(self->event.get());
  return self;
}

/* Define FlKeyboardManagerUserData */

/**
 * FlKeyboardManagerUserData:
 * The user_data used when #FlKeyboardManager sends event to
 * responders.
 */

struct _FlKeyboardManagerUserData {
  GObject parent_instance;

  // A weak reference to the owner manager.
  FlKeyboardManager* manager;
  uint64_t sequence_id;
};

G_DEFINE_TYPE(FlKeyboardManagerUserData,
              fl_keyboard_manager_user_data,
              G_TYPE_OBJECT)

static void fl_keyboard_manager_user_data_dispose(GObject* object) {
  g_return_if_fail(FL_IS_KEYBOARD_MANAGER_USER_DATA(object));
  FlKeyboardManagerUserData* self = FL_KEYBOARD_MANAGER_USER_DATA(object);
  if (self->manager != nullptr) {
    g_object_remove_weak_pointer(G_OBJECT(self->manager),
                                 reinterpret_cast<gpointer*>(&(self->manager)));
    self->manager = nullptr;
  }
}

static void fl_keyboard_manager_user_data_class_init(
    FlKeyboardManagerUserDataClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_manager_user_data_dispose;
}

static void fl_keyboard_manager_user_data_init(
    FlKeyboardManagerUserData* self) {}

// Creates a new FlKeyboardManagerUserData private class with all information.
static FlKeyboardManagerUserData* fl_keyboard_manager_user_data_new(
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

/* Define FlKeyboardManager */

struct _FlKeyboardManager {
  GObject parent_instance;

  FlKeyboardViewDelegate* view_delegate;

  // An array of #FlKeyResponder. Elements are added with
  // #fl_keyboard_manager_add_responder immediately after initialization and are
  // automatically released on dispose.
  GPtrArray* responder_list;

  // An array of #FlKeyboardPendingEvent.
  //
  // Its elements are *not* unreferenced when removed. When FlKeyboardManager is
  // disposed, this array will be set with a free_func so that the elements are
  // unreferenced when removed.
  GPtrArray* pending_responds;

  // An array of #FlKeyboardPendingEvent.
  //
  // Its elements are unreferenced when removed.
  GPtrArray* pending_redispatches;

  // The last sequence ID used. Increased by 1 by every use.
  uint64_t last_sequence_id;

  GroupLayouts group_layouts;
  std::map<uint16_t, const LayoutGoal*> keycode_to_goals;
  std::map<uint64_t, const LayoutGoal*> logical_to_mandatory_goals;
  std::set<uint16_t> goal_keycodes;
};

G_DEFINE_TYPE(FlKeyboardManager, fl_keyboard_manager, G_TYPE_OBJECT);

static void fl_keyboard_manager_dispose(GObject* object);

static void fl_keyboard_manager_class_init(FlKeyboardManagerClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_keyboard_manager_dispose;
}

static void fl_keyboard_manager_init(FlKeyboardManager* self) {
  self->group_layouts = std::map<guint8, GroupLayout>();

  self->keycode_to_goals = std::map<uint16_t, const LayoutGoal*>();
  self->logical_to_mandatory_goals = std::map<uint64_t, const LayoutGoal*>();
  self->goal_keycodes = std::set<uint16_t>();
  for (const LayoutGoal& goal : layout_goals) {
    self->keycode_to_goals[goal.keycode] = &goal;
    if (goal.mandatory) {
      self->logical_to_mandatory_goals[goal.logical_key] = &goal;
    }
    self->goal_keycodes.insert(goal.keycode);
  }

  self->responder_list = g_ptr_array_new_with_free_func(g_object_unref);

  self->pending_responds = g_ptr_array_new();
  self->pending_redispatches = g_ptr_array_new_with_free_func(g_object_unref);

  self->last_sequence_id = 1;
}

static void fl_keyboard_manager_dispose(GObject* object) {
  FlKeyboardManager* self = FL_KEYBOARD_MANAGER(object);

  if (self->view_delegate != nullptr) {
    fl_keyboard_view_delegate_subscribe_to_layout_change(self->view_delegate,
                                                        nullptr);
    g_object_remove_weak_pointer(
        G_OBJECT(self->view_delegate),
        reinterpret_cast<gpointer*>(&(self->view_delegate)));
    self->view_delegate = nullptr;
  }

  g_ptr_array_free(self->responder_list, TRUE);
  g_ptr_array_set_free_func(self->pending_responds, g_object_unref);
  g_ptr_array_free(self->pending_responds, TRUE);
  g_ptr_array_free(self->pending_redispatches, TRUE);

  G_OBJECT_CLASS(fl_keyboard_manager_parent_class)->dispose(object);
}

/* Implement FlKeyboardManager */

// This is an exact copy of g_ptr_array_find_with_equal_func.  Somehow CI
// reports that can not find symbol g_ptr_array_find_with_equal_func, despite
// the fact that it runs well locally.
static gboolean g_ptr_array_find_with_equal_func1(GPtrArray* haystack,
                                           gconstpointer needle,
                                           GEqualFunc equal_func,
                                           guint* index_) {
  guint i;
  g_return_val_if_fail(haystack != NULL, FALSE);
  if (equal_func == NULL) {
    equal_func = g_direct_equal;
  }
  for (i = 0; i < haystack->len; i++) {
    if (equal_func(g_ptr_array_index(haystack, i), needle)) {
      if (index_ != NULL) {
        *index_ = i;
      }
      return TRUE;
    }
  }

  return FALSE;
}

// Compare a #FlKeyboardPendingEvent with the given sequence_id. The needle
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

// Try to remove a pending event from `pending_redispatches` with the target
// hash.
//
// Returns true if the event is found and removed.
static bool fl_keyboard_manager_remove_redispatched(FlKeyboardManager* self,
                                                    uint64_t hash) {
  guint result_index;
  gboolean found = g_ptr_array_find_with_equal_func1(
      self->pending_redispatches, static_cast<const uint64_t*>(&hash),
      compare_pending_by_hash, &result_index);
  if (found) {
    // The removed object is freed due to `pending_redispatches`'s free_func.
    g_ptr_array_remove_index_fast(self->pending_redispatches, result_index);
    return TRUE;
  } else {
    return FALSE;
  }
}

// The callback used by a responder after the event was dispatched.
static void responder_handle_event_callback(bool handled,
                                            gpointer user_data_ptr) {
  g_return_if_fail(FL_IS_KEYBOARD_MANAGER_USER_DATA(user_data_ptr));
  FlKeyboardManagerUserData* user_data =
      FL_KEYBOARD_MANAGER_USER_DATA(user_data_ptr);
  FlKeyboardManager* self = user_data->manager;
  g_return_if_fail(self->view_delegate != nullptr);

  guint result_index = -1;
  gboolean found = g_ptr_array_find_with_equal_func1(
      self->pending_responds, &user_data->sequence_id,
      compare_pending_by_sequence_id, &result_index);
  g_return_if_fail(found);
  FlKeyboardPendingEvent* pending = FL_KEYBOARD_PENDING_EVENT(
      g_ptr_array_index(self->pending_responds, result_index));
  g_return_if_fail(pending != nullptr);
  g_return_if_fail(pending->unreplied > 0);
  pending->unreplied -= 1;
  pending->any_handled = pending->any_handled || handled;
  // All responders have replied.
  if (pending->unreplied == 0) {
    g_object_unref(user_data_ptr);
    gpointer removed =
        g_ptr_array_remove_index_fast(self->pending_responds, result_index);
    g_return_if_fail(removed == pending);
    bool should_redispatch = !pending->any_handled &&
                             !fl_keyboard_view_delegate_text_filter_key_press(
                                 self->view_delegate, pending->event.get());
    if (should_redispatch) {
      g_ptr_array_add(self->pending_redispatches, pending);
      fl_keyboard_view_delegate_redispatch_event(self->view_delegate,
                                                 std::move(pending->event));
    } else {
      g_object_unref(pending);
    }
  }
}

static uint16_t convert_key_to_char(FlKeyboardViewDelegate* view_delegate, guint keycode, gint group, gint level) {
  GdkKeymapKey key = { keycode, group, level };
  constexpr int kBmpMax = 0xD7FF;
  guint origin = fl_keyboard_view_delegate_lookup_key(view_delegate, &key);
  return origin < kBmpMax ? origin : 0xFFFF;
}

static void guarantee_layout(FlKeyboardManager* self, FlKeyEvent* event) {
  guint8 group = event->group;
  // If the current group has been built, don't need to build layout.
  if (self->group_layouts.find(group) != self->group_layouts.end()) {
    // printf("Layout for %d found\n", group);fflush(stdout);
    return;
  }
  // If the target keycode is not a goal, don't need to build layout.
  if (self->goal_keycodes.find(event->keycode) == self->goal_keycodes.end()) {
    return;
  }

  printf("Building layout for %d\n", group);fflush(stdout);
  GroupLayout& layout = self->group_layouts[group];
  printf("1\n");fflush(stdout);

  // Derive key mapping for each key code based on their layout clues.
  std::map<uint64_t, const LayoutGoal*> remaining_mandatory_goals =
      self->logical_to_mandatory_goals;

  printf("2 group %d\n", group);fflush(stdout);
#ifdef DEBUG_PRINT_LAYOUT
  std::string debug_layout_data;
  for (uint16_t keycode = 0; keycode < 128; keycode += 1) {
    std::vector<uint16_t> this_key_clues = {
        convert_key_to_char(self->view_delegate, keycode, group, 0),
        convert_key_to_char(self->view_delegate, keycode, group, 1),  // Shift
    };
    debug_format_layout_data(debug_layout_data, keycode, this_key_clues[0],
                             this_key_clues[1]);
  }
  printf("%s", debug_layout_data.c_str());
#endif
  printf("3\n");fflush(stdout);

  for (const LayoutGoal& keycode_goal : layout_goals) {
    uint16_t keycode = keycode_goal.keycode;
    std::vector<uint16_t> this_key_clues = {
        convert_key_to_char(self->view_delegate, keycode, group, 0),
        convert_key_to_char(self->view_delegate, keycode, group, 1),  // Shift
    };
    // printf("Keycode 0x%x clues 0x%x 0x%x\n", keycode, this_key_clues[0], this_key_clues[1]);fflush(stdout);
    // The logical key should be the first available clue from below:
    //
    //  - Mandatory goal, if it matches any clue. This ensures that all alnum
    //    keys can be found somewhere.
    //  - US layout, if neither clue of the key is EASCII. This ensures that
    //    there are no non-latin logical keys.
    //  - Derived on the fly from keyCode & characters.
    for (uint16_t clue : this_key_clues) {
      auto matching_goal = remaining_mandatory_goals.find(clue);
      if (matching_goal != remaining_mandatory_goals.end()) {
        // printf("Found mandatory clue 0x%x <- 0x%x\n", keycode, clue);fflush(stdout);
        // Found a key that produces a mandatory char. Use it.
        g_return_if_fail(layout[keycode] == 0);
        layout[keycode] = clue;
        remaining_mandatory_goals.erase(matching_goal);
        break;
      }
    }
    bool has_any_eascii =
        is_eascii(this_key_clues[0]) || is_eascii(this_key_clues[1]);
    // See if any produced char meets the requirement as a logical key.
    // printf("D 0x%x\n", keycode);fflush(stdout);
    if (layout[keycode] == 0 && !has_any_eascii) {
      auto found_us_layout = self->keycode_to_goals.find(keycode);
      if (found_us_layout != self->keycode_to_goals.end()) {
        // printf("Use US layout 0x%x <- 0x%lx\n", keycode, found_us_layout->second->logical_key);fflush(stdout);
        layout[keycode] = found_us_layout->second->logical_key;
      }
    }
    // printf("E\n");fflush(stdout);
  }
  // printf("3\n");fflush(stdout);

  // printf("4\n");fflush(stdout);
  // Ensure all mandatory goals are assigned.
  for (const auto mandatory_goal_iter : remaining_mandatory_goals) {
    const LayoutGoal* goal = mandatory_goal_iter.second;
    // printf("Fallback 0x%x <- 0x%lx\n", goal->keycode, goal->logical_key);fflush(stdout);
    layout[goal->keycode] = goal->logical_key;
  }

  // for (size_t i = 0; i < kLayoutSize; i += 1) {
  //   printf("k 0x%zx L 0x%lx\n", i, layout[i]);fflush(stdout);
  // }

  // debug_print_layout_result(layout);
}

FlKeyboardManager* fl_keyboard_manager_new(
    FlKeyboardViewDelegate* view_delegate) {
  // Some initialization is done in `fl_keyboard_manager_init`.
  g_return_val_if_fail(FL_IS_KEYBOARD_VIEW_DELEGATE(view_delegate), nullptr);

  FlKeyboardManager* self = FL_KEYBOARD_MANAGER(
      g_object_new(fl_keyboard_manager_get_type(), nullptr));

  self->view_delegate = view_delegate;
  g_object_add_weak_pointer(G_OBJECT(view_delegate),
                            reinterpret_cast<gpointer*>(&(self->view_delegate)));

  // The embedder responder must be added before the channel responder.
  g_ptr_array_add(
      self->responder_list,
      FL_KEY_RESPONDER(fl_key_embedder_responder_new(
          [self](const FlutterKeyEvent* event, FlutterKeyEventCallback callback,
                 void* user_data) {
            g_return_if_fail(self->view_delegate != nullptr);
            fl_keyboard_view_delegate_send_key_event(self->view_delegate, event,
                                                     callback, user_data);
          },
          &self->group_layouts)));
  g_ptr_array_add(self->responder_list,
                  FL_KEY_RESPONDER(fl_key_channel_responder_new(
                      fl_keyboard_view_delegate_get_messenger(view_delegate),
                      &self->group_layouts)));

  fl_keyboard_view_delegate_subscribe_to_layout_change(
      self->view_delegate, [self]() {
        // printf("# Group layouts cleared!\n");
        self->group_layouts.clear();
      });
  return self;
}

// The loop body to dispatch an event to a responder.
static void dispatch_to_responder(gpointer responder_data,
                                  gpointer foreach_data_ptr) {
  DispatchToResponderLoopContext* context =
      reinterpret_cast<DispatchToResponderLoopContext*>(foreach_data_ptr);
  FlKeyResponder* responder = FL_KEY_RESPONDER(responder_data);
  fl_key_responder_handle_event(responder, context->event,
                                responder_handle_event_callback,
                                context->user_data);
}

gboolean fl_keyboard_manager_handle_event(FlKeyboardManager* self,
                                          FlKeyEvent* event) {
  g_return_val_if_fail(FL_IS_KEYBOARD_MANAGER(self), FALSE);
  g_return_val_if_fail(event != nullptr, FALSE);
  g_return_val_if_fail(self->view_delegate != nullptr, FALSE);

  guarantee_layout(self, event);

  uint64_t incoming_hash = fl_keyboard_manager_get_event_hash(event);
  if (fl_keyboard_manager_remove_redispatched(self, incoming_hash)) {
    return FALSE;
  }

  printf("## Keycode 0x%x Keyval 0x%x gr 0x%x state 0x%x\n", event->keycode, event->keyval, event->group, event->state);fflush(stdout);

  FlKeyboardPendingEvent* pending = fl_keyboard_pending_event_new(
      std::unique_ptr<FlKeyEvent>(event), ++self->last_sequence_id,
      self->responder_list->len);

  g_ptr_array_add(self->pending_responds, pending);
  FlKeyboardManagerUserData* user_data =
      fl_keyboard_manager_user_data_new(self, pending->sequence_id);
  DispatchToResponderLoopContext data{
      .event = event,
      .user_data = user_data,
  };
  g_ptr_array_foreach(self->responder_list, dispatch_to_responder, &data);

  return TRUE;
}

gboolean fl_keyboard_manager_is_state_clear(FlKeyboardManager* self) {
  g_return_val_if_fail(FL_IS_KEYBOARD_MANAGER(self), FALSE);
  return self->pending_responds->len == 0 &&
         self->pending_redispatches->len == 0;
}
