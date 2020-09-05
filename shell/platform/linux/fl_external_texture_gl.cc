// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_external_texture_gl.h"

#include <EGL/egl.h>
#include <GL/gl.h>
#include <gmodule.h>

typedef struct {
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
} glFuncs;

static glFuncs* load_gl_funcs() {
  static glFuncs funcs;
  if (!funcs.valid) {
    funcs.genTextures = reinterpret_cast<void (*)(GLsizei, GLuint*)>(
        eglGetProcAddress("glGenTextures"));
    funcs.bindTexture = reinterpret_cast<void (*)(GLenum, GLuint)>(
        eglGetProcAddress("glBindTexture"));
    funcs.texParameteri = reinterpret_cast<void (*)(GLenum, GLenum, GLenum)>(
        eglGetProcAddress("glTexParameteri"));
    funcs.texImage2D =
        reinterpret_cast<void (*)(GLenum, GLint, GLint, GLsizei, GLsizei, GLint,
                                  GLenum, GLenum, const void*)>(
            eglGetProcAddress("glTexImage2D"));
    funcs.deleteTextures = reinterpret_cast<void (*)(GLsizei, const GLuint*)>(
        eglGetProcAddress("glDeleteTextures"));
    funcs.valid = true;
  }
  return &funcs;
}

struct _FlExternalTextureGl {
  GObject parent_instance;
  GLuint gl_texture_id;
  FlTextureCallback callback;
  void* user_data;
};

G_DEFINE_TYPE(FlExternalTextureGl, fl_external_texture_gl, G_TYPE_OBJECT)

static void fl_external_texture_gl_dispose(GObject* object) {
  FlExternalTextureGl* self = FL_EXTERNAL_TEXTURE_GL(object);
  glFuncs* gl = load_gl_funcs();
  gl->deleteTextures(1, &self->gl_texture_id);

  G_OBJECT_CLASS(fl_external_texture_gl_parent_class)->dispose(object);
}

static void fl_external_texture_gl_class_init(FlExternalTextureGlClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_external_texture_gl_dispose;
}

static void fl_external_texture_gl_init(FlExternalTextureGl* self) {}

int64_t fl_external_texture_gl_texture_id(FlExternalTextureGl* self) {
  g_return_val_if_fail(FL_IS_EXTERNAL_TEXTURE_GL(self), -1);
  return self->gl_texture_id;
}

gboolean fl_external_texture_gl_populate_texture(
    FlExternalTextureGl* self,
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture) {
  g_return_val_if_fail(FL_IS_EXTERNAL_TEXTURE_GL(self), FALSE);

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

  return TRUE;
}

gboolean fl_external_texture_gl_copy_pixel_buffer(FlExternalTextureGl* self,
                                                  size_t* width,
                                                  size_t* height) {
  g_return_val_if_fail(FL_IS_EXTERNAL_TEXTURE_GL(self), FALSE);

  const uint8_t* buffer;
  if (self->callback(width, height, &buffer, self->user_data) != TRUE) {
    return FALSE;
  }

  glFuncs* gl = load_gl_funcs();
  if (self->gl_texture_id == 0) {
    gl->genTextures(1, &self->gl_texture_id);
    gl->bindTexture(GL_TEXTURE_2D, self->gl_texture_id);
    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    gl->texParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    gl->bindTexture(GL_TEXTURE_2D, self->gl_texture_id);
  }
  gl->texImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, buffer);
  return TRUE;
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
