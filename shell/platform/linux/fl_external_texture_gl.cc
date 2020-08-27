// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_external_texture_gl.h"

#include <EGL/egl.h>
#include <GL/gl.h>
#include <gmodule.h>

struct {
  bool valid;
  void (*genTextures)(GLsizei n, GLuint* textures);
  void (*bindTexture)(GLenum target, GLuint texture);
  void (*texParameteri)(GLenum target, GLenum pname, GLenum param);
  void (*texImage2D)(GLenum target,
                     GLint level,
                     GLint internalformat,
                     GLsizei width,
                     GLsizei height,
                     GLint border,
                     GLenum format,
                     GLenum type,
                     const void* data);
  void (*deleteTextures)(GLsizei n, const GLuint* textures);
} gl;

struct _FlExternalTextureGl {
  GLuint gl_texture_id;
  FlTextureCallback callback;
  void* user_data;
};

G_DEFINE_TYPE(FlExternalTextureGl, fl_external_texture_gl, G_TYPE_OBJECT)

static void fl_external_texture_gl_dispose(GObject* object) {
  FlExternalTextureGl* self = FL_EXTERNAL_TEXTURE_GL(object);
  if (gl.valid) {
    gl.deleteTextures(1, &self->gl_texture_id);
  }

  G_OBJECT_CLASS(fl_external_texture_gl_parent_class)->dispose(object);
}

static void fl_external_texture_gl_class_init(FlExternalTextureGlClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_external_texture_gl_dispose;
}

static void fl_external_texture_gl_init(FlExternalTextureGl* self) {}

int64_t fl_external_texture_gl_texture_id(FlExternalTextureGl* self) {
  return reinterpret_cast<int64_t>(self);
}

bool fl_external_texture_gl_populate_texture(
    FlExternalTextureGl* self,
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture) {
  size_t real_width = width, real_height = height;
  if (!fl_external_texture_gl_copy_pixel_buffer(self, &real_width,
                                                &real_height)) {
    return false;
  }

  opengl_texture->target = GL_TEXTURE_2D;
  opengl_texture->name = self->gl_texture_id;
  opengl_texture->format = GL_RGBA8;
  opengl_texture->destruction_callback = nullptr;
  opengl_texture->user_data = nullptr;
  opengl_texture->width = real_width;
  opengl_texture->height = real_height;

  return true;
}

void fl_external_texture_gl_load_funcs(FlExternalTextureGl* self) {
  gl.genTextures = reinterpret_cast<void (*)(GLsizei, GLuint*)>(
      eglGetProcAddress("glGenTextures"));
  gl.bindTexture = reinterpret_cast<void (*)(GLenum, GLuint)>(
      eglGetProcAddress("glBindTexture"));
  gl.texParameteri = reinterpret_cast<void (*)(GLenum, GLenum, GLenum)>(
      eglGetProcAddress("glTexParameteri"));
  gl.texImage2D =
      reinterpret_cast<void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint,
                                GLenum, GLenum, const void*)>(
          eglGetProcAddress("glTexImage2D"));
  gl.deleteTextures = reinterpret_cast<void (*)(GLsizei, const GLuint*)>(
      eglGetProcAddress("glDeleteTextures"));
  gl.valid = true;
}

bool fl_external_texture_gl_copy_pixel_buffer(FlExternalTextureGl* self,
                                              size_t* width,
                                              size_t* height) {
  const FlPixelBuffer* pixel_buffer =
      self->callback(*width, *height, self->user_data);
  if (!pixel_buffer || !pixel_buffer->buffer) {
    return false;
  }
  *width = pixel_buffer->width;
  *height = pixel_buffer->height;

  if (!gl.valid) {
    fl_external_texture_gl_load_funcs(self);
  }
  if (self->gl_texture_id == 0) {
    gl.genTextures(1, &self->gl_texture_id);
    gl.bindTexture(GL_TEXTURE_2D, self->gl_texture_id);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl.texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    gl.bindTexture(GL_TEXTURE_2D, self->gl_texture_id);
  }
  gl.texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, pixel_buffer->width,
                pixel_buffer->height, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                pixel_buffer->buffer);
  return true;
}

FlExternalTextureGl* fl_external_texture_gl_new(
    FlTextureCallback texture_callback,
    void* user_data) {
  FlExternalTextureGl* self = FL_EXTERNAL_TEXTURE_GL(
      g_object_new(fl_external_texture_gl_get_type(), nullptr));

  self->callback = texture_callback;
  self->user_data = user_data;

  return self;
}
