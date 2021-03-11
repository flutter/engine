// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"

#include <gmodule.h>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/fl_texture_private.h"
#include "flutter/shell/platform/linux/fl_texture_registrar_private.h"

struct _FlTextureRegistrar {
  GObject parent_instance;

  FlEngine* engine;

  GHashTable* textures;
};

G_DEFINE_TYPE(FlTextureRegistrar, fl_texture_registrar, G_TYPE_OBJECT)

static void fl_texture_registrar_dispose(GObject* object) {
  FlTextureRegistrar* self = FL_TEXTURE_REGISTRAR(object);

  if (self->textures != nullptr) {
    g_hash_table_destroy(self->textures);
    self->textures = nullptr;
  }

  g_clear_object(&self->engine);

  G_OBJECT_CLASS(fl_texture_registrar_parent_class)->dispose(object);
}

static void fl_texture_registrar_class_init(FlTextureRegistrarClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_texture_registrar_dispose;
}

static void fl_texture_registrar_init(FlTextureRegistrar* self) {
  self->textures =
      g_hash_table_new_full(nullptr, nullptr, nullptr, g_object_unref);
}

G_MODULE_EXPORT int64_t
fl_texture_registrar_register_texture(FlTextureRegistrar* self,
                                      FlTexture* texture) {
  g_return_val_if_fail(FL_IS_TEXTURE(texture), 0);
  int64_t id = fl_texture_get_texture_id(texture);
  g_hash_table_insert(self->textures, GINT_TO_POINTER(id), texture);
  fl_engine_register_external_texture(self->engine, id);
  return id;
}

G_MODULE_EXPORT void fl_texture_registrar_mark_texture_frame_available(
    FlTextureRegistrar* self,
    int64_t texture_id) {
  g_return_if_fail(FL_IS_TEXTURE_REGISTRAR(self));
  fl_engine_mark_texture_frame_available(self->engine, texture_id);
}

bool fl_texture_registrar_populate_texture(
    FlTextureRegistrar* self,
    int64_t texture_id,
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture) {
  FlTexture* texture = FL_TEXTURE(
      g_hash_table_lookup(self->textures, GINT_TO_POINTER(texture_id)));
  if (texture == nullptr) {
    return false;
  }
  return fl_texture_populate_texture(texture, width, height, opengl_texture);
}

G_MODULE_EXPORT void fl_texture_registrar_unregister_texture(
    FlTextureRegistrar* self,
    int64_t texture_id) {
  if (!g_hash_table_remove(self->textures, GINT_TO_POINTER(texture_id))) {
    g_warning("Unregistering a non-existent texture %ld", texture_id);
    return;
  }

  fl_engine_unregister_external_texture(self->engine, texture_id);
}

FlTexture* fl_texture_registrar_get_texture(FlTextureRegistrar* registrar,
                                            int64_t texture_id) {
  return reinterpret_cast<FlTexture*>(
      g_hash_table_lookup(registrar->textures, GINT_TO_POINTER(texture_id)));
}

FlTextureRegistrar* fl_texture_registrar_new(FlEngine* engine) {
  FlTextureRegistrar* self = FL_TEXTURE_REGISTRAR(
      g_object_new(fl_texture_registrar_get_type(), nullptr));

  self->engine = FL_ENGINE(g_object_ref(engine));

  return self;
}
