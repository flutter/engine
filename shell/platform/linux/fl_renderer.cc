// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fl_renderer.h"

#include <epoxy/egl.h>
#include <epoxy/gl.h>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_backing_store_provider.h"
#include "flutter/shell/platform/linux/fl_platform_views_plugin.h"
#include "flutter/shell/platform/linux/fl_view_private.h"

G_DEFINE_QUARK(fl_renderer_error_quark, fl_renderer_error)

typedef struct {
  GObject parent_instance;

  FlView* view;

  GdkGLContext* main_context;
  GdkGLContext* resource_context;
} FlRendererPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(FlRenderer, fl_renderer, G_TYPE_OBJECT)

static void fl_renderer_class_init(FlRendererClass* klass) {}

static void fl_renderer_init(FlRenderer* self) {}

gboolean fl_renderer_start(FlRenderer* self, FlView* view, GError** error) {
  g_return_val_if_fail(FL_IS_RENDERER(self), FALSE);
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));
  priv->view = view;
  gboolean result = FL_RENDERER_GET_CLASS(self)->create_contexts(
      self, GTK_WIDGET(view), &priv->main_context, &priv->resource_context,
      error);

  if (result) {
    gdk_gl_context_set_use_es(priv->main_context, TRUE);
    gdk_gl_context_set_use_es(priv->resource_context, TRUE);

    gdk_gl_context_realize(priv->main_context, error);
    gdk_gl_context_realize(priv->resource_context, error);
  }

  if (*error != nullptr)
    return FALSE;
  return TRUE;
}

void* fl_renderer_get_proc_address(FlRenderer* self, const char* name) {
  return reinterpret_cast<void*>(eglGetProcAddress(name));
}

gboolean fl_renderer_make_current(FlRenderer* self, GError** error) {
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));
  if (priv->main_context) {
    gdk_gl_context_make_current(priv->main_context);
  }

  return TRUE;
}

gboolean fl_renderer_make_resource_current(FlRenderer* self, GError** error) {
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));
  if (priv->resource_context) {
    gdk_gl_context_make_current(priv->resource_context);
  }

  return TRUE;
}

gboolean fl_renderer_clear_current(FlRenderer* self, GError** error) {
  gdk_gl_context_clear_current();
  return TRUE;
}

guint32 fl_renderer_get_fbo(FlRenderer* self) {
  // There is only one frame buffer object - always return that.
  return 0;
}

gboolean fl_renderer_present(FlRenderer* self, GError** error) {
  return TRUE;
}

gboolean fl_renderer_create_backing_store(
    FlRenderer* self,
    const FlutterBackingStoreConfig* config,
    FlutterBackingStore* backing_store_out) {
  g_autoptr(GError) error = nullptr;
  gboolean result = fl_renderer_make_current(self, &error);
  if (!result) {
    g_warning("%s", error->message);
    return FALSE;
  }

  FlBackingStoreProvider* provider =
      fl_backing_store_provider_new(config->size.width, config->size.height);
  if (!provider) {
    g_error("Unable to create backing store");
    return FALSE;
  }

  uint32_t name = fl_backing_store_provider_get_gl_framebuffer_id(provider);
  uint32_t format = fl_backing_store_provider_get_gl_format(provider);

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeFramebuffer;
  backing_store_out->open_gl.framebuffer.user_data = provider;
  backing_store_out->open_gl.framebuffer.name = name;
  backing_store_out->open_gl.framebuffer.target = format;
  backing_store_out->open_gl.framebuffer.destruction_callback =
      [](void* user_data) {
        if (G_IS_OBJECT(user_data)) {
          g_object_unref(G_OBJECT(user_data));
        }
      };

  return TRUE;
}

gboolean fl_renderer_collect_backing_store(
    FlRenderer* self,
    const FlutterBackingStore* backing_store) {
  return TRUE;
}

// typedef struct _FlRendererSource {
//   FlGLArea* area;
//   FlGLAreaTexture* texture;
// } FlRendererSource;

// static gboolean present_layers(gpointer user_data) {
//   FlRendererSource* renderer_source =
//       reinterpret_cast<FlRendererSource*>(user_data);
//   fl_gl_area_queue_render(renderer_source->area, renderer_source->texture);
//   g_free(user_data);

//   return TRUE;
// }

gboolean fl_renderer_present_layers(FlRenderer* self,
                                    const FlutterLayer** layers,
                                    size_t layers_count) {
  FlRendererPrivate* priv = reinterpret_cast<FlRendererPrivate*>(
      fl_renderer_get_instance_private(self));
  if (!priv->view)
    return FALSE;
  fl_view_begin_frame(priv->view);

  for (size_t i = 0; i < layers_count; ++i) {
    const FlutterLayer* layer = layers[i];
    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore: {
        const FlutterBackingStore* backing_store = layer->backing_store;
        auto framebuffer = &backing_store->open_gl.framebuffer;
        g_object_ref(priv->main_context);
        FlBackingStoreProvider* data =
            reinterpret_cast<FlBackingStoreProvider*>(framebuffer->user_data);
        // fl_gl_area takes ownership of texture, so we add reference here to
        // prevent from destroying texture.
        fl_view_add_gl_area(priv->view, priv->main_context,
                            FL_GL_AREA_TEXTURE(data));
      } break;
      case kFlutterLayerContentTypePlatformView: {
        const FlutterPlatformView* platform_view = layer->platform_view;
        FlPlatformViewsPlugin* plugin =
            fl_view_get_platform_views_plugin(priv->view);
        FlPlatformView* view = fl_platform_views_plugin_get_platform_view(
            plugin, platform_view->identifier);
        GtkWidget* widget = fl_platform_view_get_view(view);
        if (!widget)
          continue;
        GdkRectangle geometry = {
            .x = static_cast<int>(layer->offset.x),
            .y = static_cast<int>(layer->offset.y),
            .width = static_cast<int>(layer->size.width),
            .height = static_cast<int>(layer->size.height),
        };
        fl_view_add_widget(priv->view, widget, &geometry);
      } break;
    }
  }

  if (layers_count > 0 &&
      layers[layers_count - 1]->type == kFlutterLayerContentTypePlatformView) {
    // We needs a widget on top of platform view to catch pointer events.
    g_object_ref(priv->main_context);
    fl_view_add_gl_area(priv->view, priv->main_context, nullptr);
  }

  fl_view_end_frame(priv->view);
  return TRUE;
}
