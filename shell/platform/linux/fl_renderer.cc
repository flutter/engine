// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fl_renderer.h"

#include <epoxy/egl.h>
#include <epoxy/gl.h>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_backing_store_provider.h"
#include "flutter/shell/platform/linux/fl_engine_private.h"
#include "flutter/shell/platform/linux/fl_view_private.h"

G_DEFINE_QUARK(fl_renderer_error_quark, fl_renderer_error)

typedef struct {
  FlView* view;

  // target dimension for resizing
  int target_width;
  int target_height;

  // whether the renderer waits for frame render
  bool blocking_main_thread;

  // true if frame was completed; resizing is not synchronized until first frame
  // was rendered
  bool had_first_frame;

  // Textures to render.
  GPtrArray* textures;
} FlRendererPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(FlRenderer, fl_renderer, G_TYPE_OBJECT)

static void fl_renderer_unblock_main_thread(FlRenderer* self) {
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));
  if (priv->blocking_main_thread) {
    priv->blocking_main_thread = false;

    FlTaskRunner* runner =
        fl_engine_get_task_runner(fl_view_get_engine(priv->view));
    fl_task_runner_release_main_thread(runner);
  }
}

static void fl_renderer_dispose(GObject* object) {
  FlRenderer* self = FL_RENDERER(object);
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));

  fl_renderer_unblock_main_thread(self);

  g_clear_pointer(&priv->textures, g_ptr_array_unref);

  G_OBJECT_CLASS(fl_renderer_parent_class)->dispose(object);
}

static void fl_renderer_class_init(FlRendererClass* klass) {
  G_OBJECT_CLASS(klass)->dispose = fl_renderer_dispose;
}

static void fl_renderer_init(FlRenderer* self) {
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));
  priv->textures = g_ptr_array_new_with_free_func(g_object_unref);
}

gboolean fl_renderer_start(FlRenderer* self, FlView* view) {
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));

  g_return_val_if_fail(FL_IS_RENDERER(self), FALSE);

  priv->view = view;
  return TRUE;
}

void* fl_renderer_get_proc_address(FlRenderer* self, const char* name) {
  g_return_val_if_fail(FL_IS_RENDERER(self), NULL);

  return reinterpret_cast<void*>(eglGetProcAddress(name));
}

void fl_renderer_make_current(FlRenderer* self) {
  g_return_if_fail(FL_IS_RENDERER(self));
  FL_RENDERER_GET_CLASS(self)->make_current(self);
}

void fl_renderer_make_resource_current(FlRenderer* self) {
  g_return_if_fail(FL_IS_RENDERER(self));
  FL_RENDERER_GET_CLASS(self)->make_resource_current(self);
}

void fl_renderer_clear_current(FlRenderer* self) {
  g_return_if_fail(FL_IS_RENDERER(self));
  FL_RENDERER_GET_CLASS(self)->clear_current(self);
}

gdouble fl_renderer_get_refresh_rate(FlRenderer* self) {
  g_return_val_if_fail(FL_IS_RENDERER(self), -1.0);
  return FL_RENDERER_GET_CLASS(self)->get_refresh_rate(self);
}

guint32 fl_renderer_get_fbo(FlRenderer* self) {
  g_return_val_if_fail(FL_IS_RENDERER(self), 0);

  // There is only one frame buffer object - always return that.
  return 0;
}

gboolean fl_renderer_create_backing_store(
    FlRenderer* renderer,
    const FlutterBackingStoreConfig* config,
    FlutterBackingStore* backing_store_out) {
  fl_renderer_make_current(renderer);

  FlBackingStoreProvider* provider =
      fl_backing_store_provider_new(config->size.width, config->size.height);
  if (!provider) {
    g_warning("Failed to create backing store");
    return FALSE;
  }

  uint32_t name = fl_backing_store_provider_get_gl_framebuffer_id(provider);
  uint32_t format = fl_backing_store_provider_get_gl_format(provider);

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeFramebuffer;
  backing_store_out->open_gl.framebuffer.user_data = provider;
  backing_store_out->open_gl.framebuffer.name = name;
  backing_store_out->open_gl.framebuffer.target = format;
  backing_store_out->open_gl.framebuffer.destruction_callback = [](void* p) {
    // Backing store destroyed in fl_renderer_collect_backing_store(), set
    // on FlutterCompositor.collect_backing_store_callback during engine start.
  };

  return TRUE;
}

gboolean fl_renderer_collect_backing_store(
    FlRenderer* self,
    const FlutterBackingStore* backing_store) {
  fl_renderer_make_current(self);

  // OpenGL context is required when destroying #FlBackingStoreProvider.
  g_object_unref(backing_store->open_gl.framebuffer.user_data);
  return TRUE;
}

void fl_renderer_wait_for_frame(FlRenderer* self,
                                int target_width,
                                int target_height) {
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));

  g_return_if_fail(FL_IS_RENDERER(self));

  priv->target_width = target_width;
  priv->target_height = target_height;

  if (priv->had_first_frame && !priv->blocking_main_thread) {
    priv->blocking_main_thread = true;
    FlTaskRunner* runner =
        fl_engine_get_task_runner(fl_view_get_engine(priv->view));
    fl_task_runner_block_main_thread(runner);
  }
}

gboolean fl_renderer_present_layers(FlRenderer* self,
                                    const FlutterLayer** layers,
                                    size_t layers_count) {
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));

  g_return_val_if_fail(FL_IS_RENDERER(self), FALSE);

  // ignore incoming frame with wrong dimensions in trivial case with just one
  // layer
  if (priv->blocking_main_thread && layers_count == 1 &&
      layers[0]->offset.x == 0 && layers[0]->offset.y == 0 &&
      (layers[0]->size.width != priv->target_width ||
       layers[0]->size.height != priv->target_height)) {
    return true;
  }

  priv->had_first_frame = true;

  fl_renderer_unblock_main_thread(self);

  if (!priv->view) {
    return FALSE;
  }

  g_ptr_array_set_size(priv->textures, 0);
  for (size_t i = 0; i < layers_count; ++i) {
    const FlutterLayer* layer = layers[i];
    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore: {
        const FlutterBackingStore* backing_store = layer->backing_store;
        auto framebuffer = &backing_store->open_gl.framebuffer;
        FlBackingStoreProvider* provider =
            FL_BACKING_STORE_PROVIDER(framebuffer->user_data);
        g_ptr_array_add(priv->textures, g_object_ref(provider));
      } break;
      case kFlutterLayerContentTypePlatformView: {
        // TODO(robert-ancell) Not implemented -
        // https://github.com/flutter/flutter/issues/41724
      } break;
    }
  }

  fl_view_redraw(priv->view);

  return TRUE;
}

void fl_renderer_render(FlRenderer* self, int width, int height) {
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));

  g_return_if_fail(FL_IS_RENDERER(self));

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);

  for (guint i = 0; i < priv->textures->len; i++) {
    FlBackingStoreProvider* texture =
        FL_BACKING_STORE_PROVIDER(g_ptr_array_index(priv->textures, i));

    uint32_t framebuffer_id =
        fl_backing_store_provider_get_gl_framebuffer_id(texture);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, framebuffer_id);
    GdkRectangle geometry = fl_backing_store_provider_get_geometry(texture);
    glBlitFramebuffer(0, 0, geometry.width, geometry.height, geometry.x,
                      geometry.y, geometry.x + geometry.width,
                      geometry.x + geometry.height, GL_COLOR_BUFFER_BIT,
                      GL_NEAREST);
  }
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

  glFlush();
}
