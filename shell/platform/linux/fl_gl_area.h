// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_GL_AREA_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_GL_AREA_H_

#include <gtk/gtk.h>

#include <glib-object.h>

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(FlGLAreaTexture,
                         fl_gl_area_texture,
                         FL,
                         GL_AREA_TEXTURE,
                         GObject)

struct _FlGLAreaTextureClass {
  GObjectClass parent_class;

  int (*get_texture)(FlGLAreaTexture* self);
};

G_DECLARE_FINAL_TYPE(FlGLArea, fl_gl_area, FL, GL_AREA, GtkWidget)

GtkWidget* fl_gl_area_new(GdkGLContext* context);

void fl_gl_area_queue_render(FlGLArea* area, FlGLAreaTexture* texture);

GdkGLContext* fl_gl_area_get_context(FlGLArea* area);

void fl_gl_area_make_current(FlGLArea* area);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_GL_AREA_H_
