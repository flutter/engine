// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_touch_manager.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"

static const int kMinTouchDeviceId = 0;
static const int kMaxTouchDeviceId = 128;

struct _FlTouchManager {
  GObject parent_instance;

  GWeakRef engine;

  FlutterViewId view_id;

  // Generates touch point IDs for touch events.
  flutter::SequentialIdGenerator* touch_id_generator;
};

G_DEFINE_TYPE(FlTouchManager, fl_touch_manager, G_TYPE_OBJECT);

static void fl_touch_manager_dispose(GObject* object) {
  FlTouchManager* self = FL_TOUCH_MANAGER(object);

  g_weak_ref_clear(&self->engine);

  G_OBJECT_CLASS(fl_touch_manager_parent_class)->dispose(object);
}

static void fl_touch_manager_class_init(FlTouchManagerClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_touch_manager_dispose;
}

static void fl_touch_manager_init(FlTouchManager* self) {}

FlTouchManager* fl_touch_manager_new(FlEngine* engine, FlutterViewId view_id) {
  g_return_val_if_fail(FL_IS_ENGINE(engine), nullptr);

  FlTouchManager* self =
      FL_TOUCH_MANAGER(g_object_new(fl_touch_manager_get_type(), nullptr));

  g_weak_ref_init(&self->engine, engine);
  self->view_id = view_id;

  self->touch_id_generator =
      new flutter::SequentialIdGenerator(kMinTouchDeviceId, kMaxTouchDeviceId);

  return self;
}

void fl_touch_manager_handle_touch_event(FlTouchManager* self,
                                         GdkEventTouch* event,
                                         gint scale_factor) {
  g_return_if_fail(FL_IS_TOUCH_MANAGER(self));

  g_autoptr(FlEngine) engine = FL_ENGINE(g_weak_ref_get(&self->engine));
  if (engine == nullptr) {
    return;
  }

  // get sequence id from GdkEvent
  GdkEventSequence* seq =
      gdk_event_get_event_sequence(reinterpret_cast<GdkEvent*>(event));
  // cast pointer to int to get unique id
  uint32_t id = reinterpret_cast<long>(seq);
  // generate touch id from unique id
  auto touch_id = self->touch_id_generator->GetGeneratedId(id);

  gdouble event_x = 0.0, event_y = 0.0;
  gdk_event_get_coords(reinterpret_cast<GdkEvent*>(event), &event_x, &event_y);

  double x = event_x * scale_factor;
  double y = event_y * scale_factor;

  GdkEventType touch_event_type =
      gdk_event_get_event_type(reinterpret_cast<GdkEvent*>(event));

  FlutterPointerEvent event_data = {};
  event_data.x = x;
  event_data.y = y;
  event_data.device_kind = kFlutterPointerDeviceKindTouch;
  event_data.device = touch_id;
  event_data.struct_size = sizeof(event_data);

  switch (touch_event_type) {
    case GDK_TOUCH_BEGIN:
      event_data.phase = FlutterPointerPhase::kDown;
      fl_engine_send_pointer_event(engine, self->view_id, event_data);
      break;
    case GDK_TOUCH_UPDATE:
      event_data.phase = FlutterPointerPhase::kMove;
      fl_engine_send_pointer_event(engine, self->view_id, event_data);
      break;
    case GDK_TOUCH_END:
      event_data.phase = FlutterPointerPhase::kUp;
      fl_engine_send_pointer_event(engine, self->view_id, event_data);

      event_data.phase = FlutterPointerPhase::kRemove;
      fl_engine_send_pointer_event(engine, self->view_id, event_data);
      self->touch_id_generator->ReleaseNumber(id);
      break;
    default:
      break;
  }
}
