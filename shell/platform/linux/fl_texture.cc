// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture.h"
#include "flutter/shell/platform/linux/fl_texture_private.h"

#include <epoxy/gl.h>
#include <gmodule.h>
#include <cstdio>

// Added here to stop the compiler from optimising this function away.
G_MODULE_EXPORT GType fl_texture_get_type();

G_DEFINE_TYPE(FlTexture, fl_texture, G_TYPE_OBJECT)

static void fl_texture_class_init(FlTextureClass* klass) {}

static void fl_texture_init(FlTexture* self) {}

gboolean fl_texture_populate_texture(FlTexture* self,
                                     uint32_t width,
                                     uint32_t height,
                                     FlutterOpenGLTexture* opengl_texture) {
  uint32_t target = 0, name = 0;
  bool populate_result = FL_TEXTURE_GET_CLASS(self)->populate_texture(
      self, &target, &name, &width, &height);
  if (!populate_result) {
    return false;
  }

  opengl_texture->target = target;
  opengl_texture->name = name;
  opengl_texture->format = GL_RGBA8;
  opengl_texture->destruction_callback = nullptr;
  opengl_texture->user_data = nullptr;
  opengl_texture->width = width;
  opengl_texture->height = height;

  return TRUE;
}

int64_t fl_texture_get_texture_id(FlTexture* self) {
  g_return_val_if_fail(FL_IS_TEXTURE(self), -1);
  return reinterpret_cast<int64_t>(self);
}
