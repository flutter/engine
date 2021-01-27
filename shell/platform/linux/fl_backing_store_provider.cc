// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fl_backing_store_provider.h"

#include <epoxy/gl.h>

struct _FlBackingStoreProvider {
  FlGLAreaTexture parent_instance;

  uint32_t framebuffer_id;
  uint32_t texture_id;
  GdkRectangle geometry;
};

G_DEFINE_TYPE(FlBackingStoreProvider,
              fl_backing_store_provider,
              fl_gl_area_texture_get_type())

static void fl_backing_store_provider_dispose(GObject* object) {
  FlBackingStoreProvider* provider = FL_BACKING_STORE_PROVIDER(object);
  fl_backing_store_provider_dealloc(provider);
  G_OBJECT_CLASS(fl_backing_store_provider_parent_class)->dispose(object);
}

static int fl_backing_store_provider_get_texture(FlGLAreaTexture* texture) {
  FlBackingStoreProvider* self = FL_BACKING_STORE_PROVIDER(texture);

  return fl_backing_store_provider_get_gl_texture_id(self);
}

static void fl_backing_store_provider_class_init(
    FlBackingStoreProviderClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_backing_store_provider_dispose;

  FL_GL_AREA_TEXTURE_CLASS(klass)->get_texture =
      fl_backing_store_provider_get_texture;
}

static void fl_backing_store_provider_init(FlBackingStoreProvider* self) {}

FlBackingStoreProvider* fl_backing_store_provider_new(int width, int height) {
  FlBackingStoreProvider* provider = FL_BACKING_STORE_PROVIDER(
      g_object_new(fl_backing_store_provider_get_type(), nullptr));

  glGenTextures(1, &provider->texture_id);
  glGenFramebuffers(1, &provider->framebuffer_id);

  glBindFramebuffer(GL_FRAMEBUFFER, provider->framebuffer_id);

  glBindTexture(GL_TEXTURE_2D, provider->texture_id);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, NULL);
  glBindTexture(GL_TEXTURE_2D, 0);

  glFramebufferTexture2D(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                         GL_TEXTURE_2D, provider->texture_id, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  printf("Status %x\n", status);

  return provider;
}

uint32_t fl_backing_store_provider_get_gl_framebuffer_id(
    FlBackingStoreProvider* provider) {
  return provider->framebuffer_id;
}

uint32_t fl_backing_store_provider_get_gl_texture_id(
    FlBackingStoreProvider* provider) {
  return provider->texture_id;
}

uint32_t fl_backing_store_provider_get_gl_target(
    FlBackingStoreProvider* provider) {
  return GL_TEXTURE_2D;
}

uint32_t fl_backing_store_provider_get_gl_format(
    FlBackingStoreProvider* provider) {
  return GL_RGBA8;
}

GdkRectangle fl_backing_store_provider_get_geometry(
    FlBackingStoreProvider* provider) {
  return provider->geometry;
}

void fl_backing_store_provider_dealloc(FlBackingStoreProvider* provider) {
  glDeleteFramebuffers(1, &provider->framebuffer_id);
  glDeleteTextures(1, &provider->texture_id);
}
