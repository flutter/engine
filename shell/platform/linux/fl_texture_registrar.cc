// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/fl_external_texture_gl.h"
#include "flutter/shell/platform/linux/fl_texture_registrar_private.h"

#include <gmodule.h>

struct _FlTextureRegistrar {
  GObject parent_instance;

  FlEngine* engine;

  GHashTable* textures;
};

// Added here to stop the compiler from optimising this function away.
G_MODULE_EXPORT GType fl_texture_registrar_get_type();

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
  self->textures = g_hash_table_new(nullptr, nullptr);
}

G_MODULE_EXPORT int64_t
fl_texture_registrar_register_texture(FlTextureRegistrar* self,
                                      FlTextureCallback texture_callback,
                                      void* user_data) {
  FlExternalTextureGl* texture =
      fl_external_texture_gl_new(texture_callback, user_data);
  int64_t id = fl_external_texture_gl_texture_id(texture);
  g_hash_table_insert(self->textures, reinterpret_cast<gpointer>(id), texture);
  fl_engine_register_external_texture(self->engine, id);
  return id;
}

G_MODULE_EXPORT void fl_texture_registrar_mark_texture_frame_available(
    FlTextureRegistrar* self,
    int64_t texture_id) {
  fl_engine_mark_texture_frame_available(self->engine, texture_id);
}

bool fl_texture_registrar_populate_texture(
    FlTextureRegistrar* self,
    int64_t texture_id,
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture) {
  FlExternalTextureGl* texture = FL_EXTERNAL_TEXTURE_GL(g_hash_table_lookup(
      self->textures, reinterpret_cast<gconstpointer>(texture_id)));
  if (texture == nullptr)
    return false;
  return fl_external_texture_gl_populate_texture(texture, width, height,
                                                 opengl_texture);
}

G_MODULE_EXPORT void fl_texture_registrar_unregister_texture(
    FlTextureRegistrar* self,
    int64_t texture_id) {
  g_hash_table_remove(self->textures, reinterpret_cast<gpointer>(texture_id));
  fl_engine_unregister_external_texture(self->engine, texture_id);
}

FlTextureRegistrar* fl_texture_registrar_new(FlEngine* engine) {
  FlTextureRegistrar* self = FL_TEXTURE_REGISTRAR(
      g_object_new(fl_texture_registrar_get_type(), nullptr));

  self->engine = FL_ENGINE(g_object_ref(engine));

  return self;
}
