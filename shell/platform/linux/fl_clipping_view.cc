// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_clipping_view.h"

#include "flutter/shell/platform/embedder/embedder.h"

struct _FlClippingView {
  GtkEventBox parent_instance;

  GdkRectangle geometry;
  GPtrArray* mutations;
};

G_DEFINE_TYPE(FlClippingView, fl_clipping_view, GTK_TYPE_EVENT_BOX)

static void fl_clipping_view_dispose(GObject* gobject) {
  FlClippingView* self = FL_CLIPPING_VIEW(gobject);

  if (self->mutations) {
    g_ptr_array_unref(self->mutations);
    self->mutations = nullptr;
  }

  G_OBJECT_CLASS(fl_clipping_view_parent_class)->dispose(gobject);
}

// Implements GtkWidget::size-allocate.
static void fl_clipping_view_size_allocate(GtkWidget* widget,
                                           GtkAllocation* allocation) {
  FlClippingView* self = FL_CLIPPING_VIEW(widget);

  gtk_widget_set_allocation(widget, allocation);

  if (gtk_widget_get_has_window(widget)) {
    if (gtk_widget_get_realized(widget))
      gdk_window_move_resize(gtk_widget_get_window(widget), allocation->x,
                             allocation->y, allocation->width,
                             allocation->height);
  }

  GtkWidget* child = gtk_bin_get_child(GTK_BIN(self));
  if (!child || !gtk_widget_get_visible(child))
    return;

  GtkAllocation child_allocation = self->geometry;
  if (!gtk_widget_get_has_window(widget)) {
    child_allocation.x += allocation->x;
    child_allocation.y += allocation->y;
  }
  gtk_widget_size_allocate(child, &child_allocation);
}

// Implements GtkWidget::draw.
static gboolean fl_clipping_view_draw(GtkWidget* widget, cairo_t* cr) {
  FlClippingView* self = FL_CLIPPING_VIEW(widget);

  // We currently can only clip widgets that have no GdkWindow.

  cairo_save(cr);
  if (self->mutations) {
    for (guint i = 0; i < self->mutations->len; i++) {
      FlutterPlatformViewMutation* mutation =
          reinterpret_cast<FlutterPlatformViewMutation*>(
              self->mutations->pdata[i]);
      switch (mutation->type) {
        case kFlutterPlatformViewMutationTypeOpacity: {
          // opacitiy is applied in fl_clipping_view_reset ().
        } break;
        case kFlutterPlatformViewMutationTypeClipRect: {
          cairo_rectangle(cr, mutation->clip_rect.left, mutation->clip_rect.top,
                          mutation->clip_rect.right - mutation->clip_rect.left,
                          mutation->clip_rect.bottom - mutation->clip_rect.top);
          cairo_clip(cr);
        } break;
        case kFlutterPlatformViewMutationTypeClipRoundedRect: {
          FlutterSize* top_left_radii =
              &mutation->clip_rounded_rect.upper_left_corner_radius;
          FlutterSize* top_right_radii =
              &mutation->clip_rounded_rect.upper_right_corner_radius;
          FlutterSize* bottom_left_radii =
              &mutation->clip_rounded_rect.lower_left_corner_radius;
          FlutterSize* bottom_right_radii =
              &mutation->clip_rounded_rect.lower_right_corner_radius;
          FlutterRect* rect = &mutation->clip_rounded_rect.rect;
          cairo_move_to(cr, rect->left + top_left_radii->width, rect->top);

          cairo_line_to(cr, rect->right - top_right_radii->width, rect->top);
          cairo_curve_to(cr, rect->right, rect->top,  //
                         rect->right, rect->top + top_right_radii->height,
                         rect->right, rect->top + top_right_radii->height);

          cairo_line_to(cr, rect->right,
                        rect->bottom - bottom_right_radii->height);
          cairo_curve_to(cr, rect->right, rect->bottom,
                         rect->right - bottom_right_radii->width, rect->bottom,
                         rect->right - bottom_right_radii->width, rect->bottom);

          cairo_line_to(cr, rect->left + bottom_left_radii->width,
                        rect->bottom);
          cairo_curve_to(cr, rect->left, rect->bottom,  //
                         rect->left, rect->bottom - bottom_left_radii->height,
                         rect->left, rect->bottom - bottom_left_radii->height);

          cairo_line_to(cr, rect->left, rect->top + top_left_radii->height);
          cairo_curve_to(cr, rect->left, rect->top,
                         rect->left + top_left_radii->width, rect->top,
                         rect->left + top_left_radii->width, rect->top);

          cairo_close_path(cr);
          cairo_clip(cr);
        } break;
        case kFlutterPlatformViewMutationTypeTransformation: {
          cairo_matrix_t matrix = {
              .xx = mutation->transformation.scaleX,
              .yx = mutation->transformation.skewY,
              .xy = mutation->transformation.skewX,
              .yy = mutation->transformation.scaleY,
              .x0 = mutation->transformation.transX,
              .y0 = mutation->transformation.transY,
          };
          cairo_transform(cr, &matrix);
        } break;
      }
    }
  }
  cairo_translate(cr, self->geometry.x, -self->geometry.y);
  gboolean result =
      GTK_WIDGET_CLASS(fl_clipping_view_parent_class)->draw(widget, cr);
  cairo_restore(cr);
  return result;
}

static void fl_clipping_view_constructed(GObject* object) {
  gtk_event_box_set_visible_window(GTK_EVENT_BOX(object), TRUE);
}

static void fl_clipping_view_class_init(FlClippingViewClass* klass) {
  GObjectClass* gobject_class = G_OBJECT_CLASS(klass);
  gobject_class->constructed = fl_clipping_view_constructed;
  gobject_class->dispose = fl_clipping_view_dispose;

  GtkWidgetClass* widget_class = GTK_WIDGET_CLASS(klass);
  widget_class->draw = fl_clipping_view_draw;
  widget_class->size_allocate = fl_clipping_view_size_allocate;
}

static void fl_clipping_view_init(FlClippingView* self) {}

GtkWidget* fl_clipping_view_new() {
  return GTK_WIDGET(g_object_new(fl_clipping_view_get_type(), nullptr));
}

void fl_clipping_view_reset(FlClippingView* self,
                            GtkWidget* child,
                            GdkRectangle* geometry,
                            GPtrArray* mutations) {
  g_return_if_fail(FL_IS_CLIPPING_VIEW(self));

  GtkWidget* old_child = gtk_bin_get_child(GTK_BIN(self));
  if (old_child != child) {
    if (old_child)
      gtk_container_remove(GTK_CONTAINER(self), old_child);

    g_object_ref(child);
    gtk_container_add(GTK_CONTAINER(self), child);
  }
  if (self->mutations) {
    g_ptr_array_unref(self->mutations);
  }
  self->geometry = *geometry;
  self->mutations = mutations;

  if (mutations) {
    for (guint i = 0; i < mutations->len; i++) {
      FlutterPlatformViewMutation* mutation =
          reinterpret_cast<FlutterPlatformViewMutation*>(
              self->mutations->pdata[i]);
      if (mutation->type == kFlutterPlatformViewMutationTypeOpacity) {
        gtk_widget_set_opacity(child, mutation->opacity);
      }
    }
  }
}
