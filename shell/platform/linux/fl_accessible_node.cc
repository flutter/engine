// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_accessible_node.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"

struct _FlAccessibleNode {
  GObjectClass parent_instance;

  // Weak reference to the engine this node is created for.
  FlEngine* engine;

  // Weak reference to the parent node of this one or %NULL.
  GtkAccessible* parent;

  // The first child of this node or %NULL.
  FlAccessibleNode* child;

  // The sibling of this node or %NULL.
  FlAccessibleNode* sibling;

  int x;
  int y;
  int width;
  int height;

  GtkATContext* at_context;
};

enum { PROP_ACCESSIBLE_ROLE = 1 };

static void fl_accessible_node_accessible_init(GtkAccessibleInterface* iface);

G_DEFINE_TYPE_WITH_CODE(
    FlAccessibleNode,
    fl_accessible_node,
    G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE(GTK_TYPE_ACCESSIBLE,
                          fl_accessible_node_accessible_init))

// GtkAccessible::get_at_context.
static GtkATContext* fl_accessible_node_accessible_get_at_context(
    GtkAccessible* accessible) {
  FlAccessibleNode* self = FL_ACCESSIBLE_NODE(accessible);
  return GTK_AT_CONTEXT(g_object_ref(self->at_context));
}

// GtkAccessible::get_platform_state.
static gboolean fl_accessible_node_accessible_get_platform_state(
    GtkAccessible* self,
    GtkAccessiblePlatformState state) {
  return FALSE;
}

// GtkAccessible::get_accessible_parent.
static GtkAccessible* fl_accessible_node_accessible_get_accessible_parent(
    GtkAccessible* accessible) {
  FlAccessibleNode* self = FL_ACCESSIBLE_NODE(accessible);
  return GTK_ACCESSIBLE(g_object_ref(self->parent));
}

// GtkAccessible::get_first_accessible_child.
static GtkAccessible* fl_accessible_node_accessible_get_first_accessible_child(
    GtkAccessible* accessible) {
  FlAccessibleNode* self = FL_ACCESSIBLE_NODE(accessible);
  return self->child != nullptr ? GTK_ACCESSIBLE(g_object_ref(self->child))
                                : nullptr;
}

// GtkAccessible::get_next_accessible_sibling.
static GtkAccessible* fl_accessible_node_accessible_get_next_accessible_sibling(
    GtkAccessible* accessible) {
  FlAccessibleNode* self = FL_ACCESSIBLE_NODE(accessible);
  return self->sibling != nullptr ? GTK_ACCESSIBLE(g_object_ref(self->sibling))
                                  : nullptr;
}

// GtkAccessible::get_bounds.
static gboolean fl_accessible_node_accessible_get_bounds(
    GtkAccessible* accessible,
    int* x,
    int* y,
    int* width,
    int* height) {
  FlAccessibleNode* self = FL_ACCESSIBLE_NODE(accessible);
  *x = self->x;
  *y = self->y;
  *width = self->width;
  *height = self->height;
  return TRUE;
}

static void fl_accessible_node_accessible_init(GtkAccessibleInterface* iface) {
  iface->get_at_context = fl_accessible_node_accessible_get_at_context;
  iface->get_platform_state = fl_accessible_node_accessible_get_platform_state;
  iface->get_accessible_parent =
      fl_accessible_node_accessible_get_accessible_parent;
  iface->get_first_accessible_child =
      fl_accessible_node_accessible_get_first_accessible_child;
  iface->get_next_accessible_sibling =
      fl_accessible_node_accessible_get_next_accessible_sibling;
  iface->get_bounds = fl_accessible_node_accessible_get_bounds;
}

static void fl_accessible_node_get_property(GObject* object,
                                            guint prop_id,
                                            GValue* value,
                                            GParamSpec* pspec) {
  switch (prop_id) {
    case PROP_ACCESSIBLE_ROLE:
      g_value_set_enum(value, GTK_ACCESSIBLE_ROLE_GENERIC);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void fl_accessible_node_set_property(GObject* object,
                                            guint prop_id,
                                            const GValue* value,
                                            GParamSpec* pspec) {
  switch (prop_id) {
    case PROP_ACCESSIBLE_ROLE:
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}

static void fl_accessible_node_dispose(GObject* object) {
  FlAccessibleNode* self = FL_ACCESSIBLE_NODE(object);

  if (self->engine != nullptr) {
    g_object_remove_weak_pointer(object,
                                 reinterpret_cast<gpointer*>(&(self->engine)));
    self->engine = nullptr;
  }
  g_clear_object(&self->at_context);
  if (self->parent != nullptr) {
    g_object_remove_weak_pointer(object,
                                 reinterpret_cast<gpointer*>(&(self->parent)));
    self->parent = nullptr;
  }
  g_clear_object(&self->child);
  g_clear_object(&self->sibling);

  G_OBJECT_CLASS(fl_accessible_node_parent_class)->dispose(object);
}

static void fl_accessible_node_class_init(FlAccessibleNodeClass* klass) {
  GObjectClass* object_class = G_OBJECT_CLASS(klass);
  object_class->get_property = fl_accessible_node_get_property;
  object_class->set_property = fl_accessible_node_set_property;
  object_class->dispose = fl_accessible_node_dispose;

  g_object_class_override_property(object_class, PROP_ACCESSIBLE_ROLE,
                                   "accessible-role");
}

static void fl_accessible_node_init(FlAccessibleNode* self) {}

FlAccessibleNode* fl_accessible_node_new(GdkDisplay* display,
                                         FlEngine* engine) {
  FlAccessibleNode* self =
      FL_ACCESSIBLE_NODE(g_object_new(fl_accessible_node_get_type(), nullptr));
  self->engine = engine;
  g_object_add_weak_pointer(G_OBJECT(self),
                            reinterpret_cast<gpointer*>(&self->engine));
  self->at_context = gtk_at_context_create(GTK_ACCESSIBLE_ROLE_GENERIC,
                                           GTK_ACCESSIBLE(self), display);
  return self;
}

void fl_accessible_node_set_parent(FlAccessibleNode* self,
                                   GtkAccessible* parent) {
  g_return_if_fail(FL_IS_ACCESSIBLE_NODE(self));

  if (self->parent != nullptr) {
    g_object_remove_weak_pointer(G_OBJECT(self),
                                 reinterpret_cast<gpointer*>(&(self->parent)));
  }
  self->parent = parent;
  g_object_add_weak_pointer(G_OBJECT(self),
                            reinterpret_cast<gpointer*>(&(self->parent)));
}

void fl_accessible_node_set_first_child(FlAccessibleNode* self,
                                        FlAccessibleNode* child) {
  g_return_if_fail(FL_IS_ACCESSIBLE_NODE(self));

  g_clear_object(&self->child);
  self->child =
      child != nullptr ? FL_ACCESSIBLE_NODE(g_object_ref(child)) : nullptr;
}

void fl_accessible_node_set_sibling(FlAccessibleNode* self,
                                    FlAccessibleNode* sibling) {
  g_return_if_fail(FL_IS_ACCESSIBLE_NODE(self));

  g_clear_object(&self->sibling);
  self->sibling =
      sibling != nullptr ? FL_ACCESSIBLE_NODE(g_object_ref(sibling)) : nullptr;
}

void fl_accessible_node_set_bounds(FlAccessibleNode* self,
                                   gint x,
                                   gint y,
                                   gint width,
                                   gint height) {
  g_return_if_fail(FL_IS_ACCESSIBLE_NODE(self));

  self->x = x;
  self->y = y;
  self->width = width;
  self->height = height;
}
