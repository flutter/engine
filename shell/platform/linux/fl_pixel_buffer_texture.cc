// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_pixel_buffer_texture.h"

#include <epoxy/gl.h>
#include <gmodule.h>
#include <cstdio>

#define GL_CALL(expr)                                                      \
  do {                                                                     \
    expr;                                                                  \
    GLenum err = glGetError();                                             \
    if (err) {                                                             \
      fprintf(stderr, "glGetError %x (%s:%d)\n", err, __FILE__, __LINE__); \
    }                                                                      \
  } while (0);

typedef struct {
  GLuint texture_id;
} FlPixelBufferTexturePrivate;

// Added here to stop the compiler from optimising this function away.
G_MODULE_EXPORT GType fl_pixel_buffer_texture_get_type();

G_DEFINE_TYPE_WITH_PRIVATE(FlPixelBufferTexture,
                           fl_pixel_buffer_texture,
                           fl_texture_get_type())

static void fl_pixel_buffer_texture_dispose(GObject* object) {
  FlPixelBufferTexture* self = FL_PIXEL_BUFFER_TEXTURE(object);
  FlPixelBufferTexturePrivate* priv =
      reinterpret_cast<FlPixelBufferTexturePrivate*>(
          fl_pixel_buffer_texture_get_instance_private(self));

  if (priv->texture_id) {
    glDeleteTextures(1, &priv->texture_id);
    priv->texture_id = 0;
  }

  G_OBJECT_CLASS(fl_pixel_buffer_texture_parent_class)->dispose(object);
}

static gboolean fl_pixel_buffer_texture_populate(FlTexture* texture,
                                                 uint32_t* target,
                                                 uint32_t* name,
                                                 uint32_t* width,
                                                 uint32_t* height,
                                                 GError** error) {
  FlPixelBufferTexture* self = FL_PIXEL_BUFFER_TEXTURE(texture);
  FlPixelBufferTexturePrivate* priv =
      reinterpret_cast<FlPixelBufferTexturePrivate*>(
          fl_pixel_buffer_texture_get_instance_private(self));

  uint32_t format = 0;
  const uint8_t* buffer = nullptr;
  if (!FL_PIXEL_BUFFER_TEXTURE_GET_CLASS(self)->copy_pixels(
          self, &buffer, &format, width, height, error)) {
    return FALSE;
  }
  if (priv->texture_id == 0) {
    GL_CALL(glGenTextures(1, &priv->texture_id));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, priv->texture_id));
    GL_CALL(
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    GL_CALL(
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  } else {
    GL_CALL(glBindTexture(GL_TEXTURE_2D, priv->texture_id));
  }
  *target = GL_TEXTURE_2D;
  *name = priv->texture_id;
  GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, *width, *height, 0, format,
                       GL_UNSIGNED_BYTE, buffer));
  return TRUE;
}

static void fl_pixel_buffer_texture_class_init(
    FlPixelBufferTextureClass* klass) {
  FL_TEXTURE_CLASS(klass)->populate = fl_pixel_buffer_texture_populate;

  G_OBJECT_CLASS(klass)->dispose = fl_pixel_buffer_texture_dispose;
}

static void fl_pixel_buffer_texture_init(FlPixelBufferTexture* self) {}
