// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fl_renderer_headless.h"

struct _FlRendererHeadless {
  FlRenderer parent_instance;
};

G_DEFINE_TYPE(FlRendererHeadless, fl_renderer_headless, fl_renderer_get_type())

static gboolean fl_renderer_headless_create_contexts(FlRenderer* renderer,
                                                     GtkWidget* widget,
                                                     GdkGLContext** visible,
                                                     GdkGLContext** resource,
                                                     GError** error) {
  return FALSE;
}

static void fl_renderer_headless_class_init(FlRendererHeadlessClass* klass) {
  FL_RENDERER_CLASS(klass)->create_contexts =
      fl_renderer_headless_create_contexts;
}

static void fl_renderer_headless_init(FlRendererHeadless* self) {}

FlRenderer* fl_renderer_headless_new() {
  return FL_RENDERER(g_object_new(fl_renderer_headless_get_type(), nullptr));
}
