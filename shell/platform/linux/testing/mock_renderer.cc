// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/testing/mock_renderer.h"

struct _FlMockRenderer {
  FlRenderer parent_instance;
};

G_DEFINE_TYPE(FlMockRenderer, fl_mock_renderer, fl_renderer_get_type())

// Implements FlRenderer::create_contexts.
static gboolean fl_mock_renderer_create_contexts(FlRenderer* renderer,
                                                 GtkWidget* widget,
                                                 GdkGLContext** visible,
                                                 GdkGLContext** resource,
                                                 GError** error) {
  return TRUE;
}

static void fl_mock_renderer_class_init(FlMockRendererClass* klass) {
  FL_RENDERER_CLASS(klass)->create_contexts = fl_mock_renderer_create_contexts;
}

static void fl_mock_renderer_init(FlMockRenderer* self) {}

// Creates a stub renderer
FlRenderer* fl_mock_renderer_new() {
  return FL_RENDERER(g_object_new(fl_mock_renderer_get_type(), nullptr));
}
