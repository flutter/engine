// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_accessible_application.h"

struct _FlAccessibleApplication {
  AtkObject parent_instance;

  AtkObject* child;
};

static void fl_accessible_application_window_interface_init(
    AtkWindowIface* iface);

G_DEFINE_TYPE_WITH_CODE(
    FlAccessibleApplication,
    fl_accessible_application,
    ATK_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(ATK_TYPE_WINDOW,
                          fl_accessible_application_window_interface_init))

// Implements AtkObject::get_name.
static const gchar* fl_accessible_application_get_name(AtkObject* accessible) {
  return g_get_prgname();
}

// Implements AtkObject::get_n_children.
static gint fl_accessible_application_get_n_children(AtkObject* accessible) {
  FlAccessibleApplication* self = FL_ACCESSIBLE_APPLICATION(accessible);
  return self->child != nullptr ? 1 : 0;
}

// Implements AtkObject::ref_child.
static AtkObject* fl_accessible_application_ref_child(AtkObject* accessible,
                                                      gint i) {
  FlAccessibleApplication* self = FL_ACCESSIBLE_APPLICATION(accessible);
  return i == 0 && self->child != nullptr
             ? ATK_OBJECT(g_object_ref(self->child))
             : nullptr;
}

// Implements AtkObject::get_role.
static AtkRole fl_accessible_application_get_role(AtkObject* accessible) {
  return ATK_ROLE_APPLICATION;
}

static void fl_accessible_application_class_init(
    FlAccessibleApplicationClass* klass) {
  ATK_OBJECT_CLASS(klass)->get_name = fl_accessible_application_get_name;
  ATK_OBJECT_CLASS(klass)->get_n_children =
      fl_accessible_application_get_n_children;
  ATK_OBJECT_CLASS(klass)->ref_child = fl_accessible_application_ref_child;
  ATK_OBJECT_CLASS(klass)->get_role = fl_accessible_application_get_role;
}

static void fl_accessible_application_window_interface_init(
    AtkWindowIface* iface) {}

static void fl_accessible_application_init(FlAccessibleApplication* self) {}

FlAccessibleApplication* fl_accessible_application_new() {
  return FL_ACCESSIBLE_APPLICATION(
      g_object_new(fl_accessible_application_get_type(), nullptr));
}
