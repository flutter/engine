// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_touch_manager.h"

static constexpr int kMicrosecondsPerMillisecond = 1000;
static const int kMinTouchDeviceId = 0;
static const int kMaxTouchDeviceId = 128;


struct _FlTouchManager {
  GObject parent_instance;

  GWeakRef view_delegate;

  // Generates touch point IDs for touch events.
  flutter::SequentialIdGenerator touch_id_generator{kMinTouchDeviceId,
                                                   kMaxTouchDeviceId};
};

G_DEFINE_TYPE(FlTouchManager, fl_touch_manager, G_TYPE_OBJECT);

static void fl_touch_manager_dispose(GObject* object) {
  FlTouchManager* self = FL_TOUCH_MANAGER(object);

  g_weak_ref_clear(&self->view_delegate);

  G_OBJECT_CLASS(fl_touch_manager_parent_class)->dispose(object);
}

static void fl_touch_manager_class_init(FlTouchManagerClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_touch_manager_dispose;
}

static void fl_touch_manager_init(FlTouchManager* self) {}

FlTouchManager* fl_touch_manager_new(
    FlTouchViewDelegate* view_delegate) {
  g_return_val_if_fail(FL_IS_TOUCH_VIEW_DELEGATE(view_delegate), nullptr);

  FlTouchManager* self = FL_TOUCH_MANAGER(
      g_object_new(fl_touch_manager_get_type(), nullptr));

  g_weak_ref_init(&self->view_delegate, view_delegate);

  return self;
}

void fl_touch_manager_handle_touch_event(FlTouchManager* self,
                                         GdkEventTouch* event,
                                         gint scale_factor) {
  g_return_if_fail(FL_IS_TOUCH_MANAGER(self));
  g_autoptr(FlTouchViewDelegate) view_delegate =
      FL_TOUCH_VIEW_DELEGATE(g_weak_ref_get(&self->view_delegate));
  if (view_delegate == nullptr) {
    return;
  }

  // get sequence id from GdkEvent
  GdkEventSequence* seq =
      gdk_event_get_event_sequence(reinterpret_cast<GdkEvent*>(event));
  // cast pointer to int to get unique id
  uint32_t id = reinterpret_cast<long>(seq);
  // generate touch id from unique id
  auto touch_id = self->touch_id_generator.GetGeneratedId(id);

  gdouble event_x = 0.0, event_y = 0.0;
  gdk_event_get_coords(reinterpret_cast<GdkEvent*>(event), &event_x, &event_y);

  double x = event_x * scale_factor;
  double y = event_y * scale_factor;

  GdkEventType touch_event_type =
      gdk_event_get_event_type(reinterpret_cast<GdkEvent*>(event));
  switch (touch_event_type) {
    case GDK_TOUCH_BEGIN:
      OnPointerDown(
          self->engine, self->view_id, x, y, kFlutterPointerDeviceKindTouch,
          touch_id,
          FlutterPointerMouseButtons::kFlutterPointerButtonMousePrimary);
      break;
    case GDK_TOUCH_UPDATE:
      OnPointerMove(self->engine, self->view_id, x, y,
                    kFlutterPointerDeviceKindTouch, touch_id, 0);
      break;
    case GDK_TOUCH_END:
      OnPointerUp(
          self->engine, self->view_id, x, y, kFlutterPointerDeviceKindTouch,
          touch_id,
          FlutterPointerMouseButtons::kFlutterPointerButtonMousePrimary);
      OnPointerLeave(self->engine, self->view_id, x, y,
                     kFlutterPointerDeviceKindTouch, touch_id);
      self->touch_id_generator.ReleaseNumber(id);
      break;
    default:
      break;
  }
}

