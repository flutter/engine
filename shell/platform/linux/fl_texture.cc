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

#define GL_CALL(expr)                                                      \
  do {                                                                     \
    expr;                                                                  \
    GLenum err = glGetError();                                             \
    if (err) {                                                             \
      fprintf(stderr, "glGetError %x (%s:%d)\n", err, __FILE__, __LINE__); \
    }                                                                      \
  } while (0);

struct _FlPixelBufferTexture {
  FlTexture parent_instance;

  GLuint texture_id;

  FlCopyPixelBufferCallback callback;
  gpointer user_data;
  GDestroyNotify destroy_notify;
};

G_DEFINE_TYPE(FlPixelBufferTexture,
              fl_pixel_buffer_texture,
              fl_texture_get_type())

static void fl_pixel_buffer_texture_dispose(GObject* object) {
  FlPixelBufferTexture* self = FL_PIXEL_BUFFER_TEXTURE(object);

  if (self->destroy_notify) {
    self->destroy_notify(self->user_data);
  }

  if (self->texture_id) {
    glDeleteTextures(1, &self->texture_id);
    self->texture_id = 0;
  }

  G_OBJECT_CLASS(fl_pixel_buffer_texture_parent_class)->dispose(object);
}

static gboolean fl_pixel_buffer_texture_populate_texture(FlTexture* texture,
                                                         uint32_t* target,
                                                         uint32_t* name,
                                                         uint32_t* width,
                                                         uint32_t* height) {
  FlPixelBufferTexture* self = FL_PIXEL_BUFFER_TEXTURE(texture);
  uint32_t format = 0;
  const uint8_t* buffer = nullptr;
  if (!self->callback ||
      !self->callback(&buffer, &format, width, height, self->user_data)) {
    return FALSE;
  }
  if (self->texture_id == 0) {
    GL_CALL(glGenTextures(1, &self->texture_id));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, self->texture_id));
    GL_CALL(
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER));
    GL_CALL(
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
  } else {
    GL_CALL(glBindTexture(GL_TEXTURE_2D, self->texture_id));
  }
  *target = GL_TEXTURE_2D;
  *name = self->texture_id;
  GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, *width, *height, 0, format,
                       GL_UNSIGNED_BYTE, buffer));
  return TRUE;
}

static void fl_pixel_buffer_texture_class_init(
    FlPixelBufferTextureClass* klass) {
  FL_TEXTURE_CLASS(klass)->populate_texture =
      fl_pixel_buffer_texture_populate_texture;

  G_OBJECT_CLASS(klass)->dispose = fl_pixel_buffer_texture_dispose;
}

static void fl_pixel_buffer_texture_init(FlPixelBufferTexture* self) {}

G_MODULE_EXPORT FlPixelBufferTexture* fl_pixel_buffer_texture_new(
    FlCopyPixelBufferCallback callback,
    void* user_data,
    GDestroyNotify destroy_notify) {
  g_return_val_if_fail(callback != nullptr, nullptr);

  FlPixelBufferTexture* texture = reinterpret_cast<FlPixelBufferTexture*>(
      g_object_new(fl_pixel_buffer_texture_get_type(), nullptr));
  texture->callback = callback;
  texture->user_data = user_data;
  texture->destroy_notify = destroy_notify;
  return texture;
}

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
