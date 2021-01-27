// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fl_renderer_gl.h"

struct _FlRendererGL {
  FlRenderer parent_instance;
};

G_DEFINE_TYPE(FlRendererGL, fl_renderer_gl, fl_renderer_get_type())

static gboolean fl_renderer_gl_create_contexts(FlRenderer* renderer,
                                               GtkWidget* widget,
                                               GdkGLContext** visible,
                                               GdkGLContext** resource,
                                               GError** error) {
  GdkWindow* window = gtk_widget_get_parent_window(widget);

  *visible = gdk_window_create_gl_context(window, error);
  *resource = gdk_window_create_gl_context(window, error);

  if (*error != nullptr)
    return FALSE;
  return TRUE;
}

static void fl_renderer_gl_class_init(FlRendererGLClass* klass) {
  FL_RENDERER_CLASS(klass)->create_contexts = fl_renderer_gl_create_contexts;
}

static void fl_renderer_gl_init(FlRendererGL* self) {}

FlRenderer* fl_renderer_gl_new() {
  return FL_RENDERER(g_object_new(fl_renderer_gl_get_type(), nullptr));
}
