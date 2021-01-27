// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_gl_area.h"

#include <epoxy/gl.h>

G_DEFINE_TYPE(FlGLAreaTexture, fl_gl_area_texture, G_TYPE_OBJECT)

static void fl_gl_area_texture_class_init(FlGLAreaTextureClass* klass) {}
static void fl_gl_area_texture_init(FlGLAreaTexture* self) {}

struct _FlGLArea {
  GtkWidget parent_instance;

  GdkGLContext* context;

  FlGLAreaTexture* texture;
};

G_DEFINE_TYPE(FlGLArea, fl_gl_area, GTK_TYPE_WIDGET)

static void fl_gl_area_dispose(GObject* gobject) {
  FlGLArea* area = FL_GL_AREA(gobject);

  g_clear_object(&area->context);
  g_clear_object(&area->texture);

  G_OBJECT_CLASS(fl_gl_area_parent_class)->dispose(gobject);
}

static void fl_gl_area_realize(GtkWidget* widget) {
  GtkAllocation allocation;
  gtk_widget_get_allocation(widget, &allocation);

  gtk_widget_set_realized(widget, TRUE);

  GdkWindowAttr attributes;
  attributes.window_type = GDK_WINDOW_CHILD;
  attributes.x = allocation.x;
  attributes.y = allocation.y;
  attributes.width = allocation.width;
  attributes.height = allocation.height;
  attributes.wclass = GDK_INPUT_OUTPUT;
  attributes.visual = gtk_widget_get_visual(widget);
  attributes.event_mask = gtk_widget_get_events(widget);
  gint attributes_mask = GDK_WA_X | GDK_WA_Y | GDK_WA_VISUAL;
  GdkWindow* window = gdk_window_new(gtk_widget_get_parent_window(widget),
                                     &attributes, attributes_mask);
  gtk_widget_set_window(widget, window);
  gtk_widget_register_window(widget, window);
}

static void fl_gl_area_delete_buffers(FlGLArea* area) {
  g_clear_object(&area->texture);
}

static void fl_gl_area_unrealize(GtkWidget* widget) {
  FlGLArea* area = FL_GL_AREA(widget);
  fl_gl_area_make_current(area);
  fl_gl_area_delete_buffers(area);

  /* Make sure to unset the context if current */
  if (area->context == gdk_gl_context_get_current())
    gdk_gl_context_clear_current();

  GTK_WIDGET_CLASS(fl_gl_area_parent_class)->unrealize(widget);
}

static void fl_gl_area_size_allocate(GtkWidget* widget,
                                     GtkAllocation* allocation) {
  gtk_widget_set_allocation(widget, allocation);

  if (gtk_widget_get_has_window(widget)) {
    if (gtk_widget_get_realized(widget))
      gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x,
                             allocation->y, allocation->width,
                             allocation->height);
  }
}

static gboolean fl_gl_area_draw(GtkWidget* widget, cairo_t* cr) {
  FlGLArea* area = FL_GL_AREA(widget);
  int w, h, scale;

  fl_gl_area_make_current(area);

  scale = gtk_widget_get_scale_factor(widget);
  w = gtk_widget_get_allocated_width(widget) * scale;
  h = gtk_widget_get_allocated_height(widget) * scale;

  if (area->texture) {
    gdk_cairo_draw_from_gl(
        cr, gtk_widget_get_window(widget),
        FL_GL_AREA_TEXTURE_GET_CLASS(area->texture)->get_texture(area->texture),
        GL_TEXTURE, scale, 0, 0, w, h);
    fl_gl_area_make_current(area);
  }

  return TRUE;
}

static void fl_gl_area_class_init(FlGLAreaClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->dispose = fl_gl_area_dispose;

  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->realize = fl_gl_area_realize;
  widget_class->unrealize = fl_gl_area_unrealize;
  widget_class->size_allocate = fl_gl_area_size_allocate;
  widget_class->draw = fl_gl_area_draw;

  gtk_widget_class_set_accessible_role(widget_class, ATK_ROLE_DRAWING_AREA);
}

static void fl_gl_area_init(FlGLArea* area) {
  gtk_widget_set_can_focus(GTK_WIDGET(area), TRUE);
  gtk_widget_set_app_paintable(GTK_WIDGET(area), TRUE);
}

GtkWidget* fl_gl_area_new(GdkGLContext* context) {
  g_return_val_if_fail(GDK_IS_GL_CONTEXT(context), nullptr);
  FlGLArea* area =
      reinterpret_cast<FlGLArea*>(g_object_new(fl_gl_area_get_type(), nullptr));
  area->context = context;
  return GTK_WIDGET(area);
}

void fl_gl_area_queue_render(FlGLArea* area, FlGLAreaTexture* texture) {
  g_return_if_fail(FL_IS_GL_AREA(area));

  g_clear_object(&area->texture);
  g_set_object(&area->texture, texture);

  gtk_widget_queue_draw(GTK_WIDGET(area));
}

GdkGLContext* fl_gl_area_get_context(FlGLArea* area) {
  g_return_val_if_fail(FL_IS_GL_AREA(area), NULL);

  return area->context;
}

void fl_gl_area_make_current(FlGLArea* area) {
  GtkWidget* widget;

  g_return_if_fail(FL_IS_GL_AREA(area));

  widget = GTK_WIDGET(area);

  g_return_if_fail(gtk_widget_get_realized(widget));

  gdk_gl_context_make_current(area->context);
}
