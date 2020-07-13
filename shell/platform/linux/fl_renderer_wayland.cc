// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fl_renderer_wayland.h"

#include <wayland-egl-core.h>
#include <string>

static wl_registry* registry_global = nullptr;
static wl_subcompositor* subcompositor_global = nullptr;

static void registry_handle_global(void*,
                                   wl_registry* registry,
                                   uint32_t id,
                                   const char* name,
                                   uint32_t max_version) {
  if (std::string{name} == std::string{wl_subcompositor_interface.name}) {
    const wl_interface* interface = &wl_subcompositor_interface;
    uint32_t version =
        std::min(static_cast<uint32_t>(interface->version), max_version);
    subcompositor_global = static_cast<wl_subcompositor*>(
        wl_registry_bind(registry, id, interface, version));
  }
}

static void registry_handle_global_remove(void*, wl_registry*, uint32_t) {}

static const wl_registry_listener registry_listener = {
    .global = registry_handle_global,
    .global_remove = registry_handle_global_remove,
};

static void lazy_initialize_wayland_globals() {
  if (registry_global)
    return;

  GdkWaylandDisplay* gdk_display =
      GDK_WAYLAND_DISPLAY(gdk_display_get_default());
  g_return_if_fail(gdk_display);

  wl_display* display = gdk_wayland_display_get_wl_display(gdk_display);
  registry_global = wl_display_get_registry(display);
  wl_registry_add_listener(registry_global, &registry_listener, nullptr);
  wl_display_roundtrip(display);
}

struct _FlRendererWayland {
  FlRenderer parent_instance;

  GdkWindow* toplevel_window;

  struct {
    wl_subsurface* subsurface;
    wl_surface* surface;
    wl_egl_window* egl_window;
    GdkRectangle geometry;
    gint scale;
  } subsurface;

  // The resource surface will not be mapped, but needs to be a wl_surface
  // because ONLY window EGL surfaces are supported on Wayland
  struct {
    wl_surface* surface;
    wl_egl_window* egl_window;
  } resource;
};

G_DEFINE_TYPE(FlRendererWayland, fl_renderer_wayland, fl_renderer_get_type())

static void fl_renderer_wayland_set_window(FlRenderer* renderer,
                                           GdkWindow* window) {
  FlRendererWayland* self = FL_RENDERER_WAYLAND(renderer);
  g_return_if_fail(GDK_IS_WAYLAND_WINDOW(window));
  self->toplevel_window = GDK_WAYLAND_WINDOW(window);
}

// Implements FlRenderer::create_display
static EGLDisplay fl_renderer_wayland_create_display(FlRenderer* /*renderer*/) {
  GdkWaylandDisplay* gdk_display =
      GDK_WAYLAND_DISPLAY(gdk_display_get_default());
  g_return_val_if_fail(gdk_display, nullptr);
  return eglGetDisplay(gdk_wayland_display_get_wl_display(gdk_display));
}

static EGLSurfacePair fl_renderer_wayland_create_surface(FlRenderer* renderer,
                                                         EGLDisplay egl_display,
                                                         EGLConfig config) {
  static const EGLSurfacePair null_result{EGL_NO_SURFACE, EGL_NO_SURFACE};

  FlRendererWayland* self = FL_RENDERER_WAYLAND(renderer);

  GdkWaylandDisplay* gdk_display =
      GDK_WAYLAND_DISPLAY(gdk_display_get_default());
  g_return_val_if_fail(gdk_display, null_result);

  wl_compositor* compositor =
      gdk_wayland_display_get_wl_compositor(gdk_display);
  g_return_val_if_fail(compositor, null_result);

  self->subsurface.surface = wl_compositor_create_surface(compositor);
  self->resource.surface = wl_compositor_create_surface(compositor);
  g_return_val_if_fail(self->subsurface.surface, null_result);
  g_return_val_if_fail(self->resource.surface, null_result);
  self->subsurface.geometry = GdkRectangle{0, 0, 1, 1};
  self->subsurface.scale = 1;

  self->subsurface.egl_window =
      wl_egl_window_create(self->subsurface.surface, 1, 1);
  self->resource.egl_window =
      wl_egl_window_create(self->resource.surface, 1, 1);
  g_return_val_if_fail(self->subsurface.egl_window, null_result);
  g_return_val_if_fail(self->resource.egl_window, null_result);

  EGLSurface visible = eglCreateWindowSurface(
      egl_display, config, self->subsurface.egl_window, nullptr);
  EGLSurface resource = eglCreateWindowSurface(
      egl_display, config, self->resource.egl_window, nullptr);
  g_return_val_if_fail(visible != EGL_NO_SURFACE, null_result);
  g_return_val_if_fail(resource != EGL_NO_SURFACE, null_result);

  lazy_initialize_wayland_globals();
  g_return_val_if_fail(subcompositor_global, null_result);

  wl_surface* toplevel_surface =
      gdk_wayland_window_get_wl_surface(self->toplevel_window);
  g_return_val_if_fail(toplevel_surface, null_result);

  self->subsurface.subsurface = wl_subcompositor_get_subsurface(
      subcompositor_global, self->subsurface.surface, toplevel_surface);
  g_return_val_if_fail(self->subsurface.subsurface, null_result);
  wl_subsurface_set_desync(self->subsurface.subsurface);
  wl_subsurface_set_position(self->subsurface.subsurface, 0, 0);
  wl_surface_commit(self->subsurface.surface);

  // Give the subsurface an empty input region so the main surface gets input
  wl_region* region = wl_compositor_create_region(compositor);
  wl_surface_set_input_region(self->subsurface.surface, region);
  wl_region_destroy(region);
  wl_surface_commit(self->subsurface.surface);

  return EGLSurfacePair{visible, resource};
}

static void fl_renderer_wayland_set_geometry(FlRenderer* renderer,
                                             GdkRectangle* geometry,
                                             gint scale) {
  FlRendererWayland* self = FL_RENDERER_WAYLAND(renderer);

  if (!self->subsurface.egl_window || !self->subsurface.surface) {
    return;
  }

  if (scale != self->subsurface.scale) {
    wl_surface_set_buffer_scale(self->subsurface.surface, scale);
  }

  // NOTE: position is unscaled but size is scaled

  if (geometry->x != self->subsurface.geometry.x ||
      geometry->y != self->subsurface.geometry.y) {
    wl_subsurface_set_position(self->subsurface.subsurface, geometry->x,
                               geometry->y);
  }

  if (geometry->width != self->subsurface.geometry.width ||
      geometry->height != self->subsurface.geometry.height ||
      scale != self->subsurface.scale) {
    wl_egl_window_resize(self->subsurface.egl_window, geometry->width * scale,
                         geometry->height * scale, 0, 0);
  }

  self->subsurface.geometry = *geometry;
  self->subsurface.scale = scale;
}

static void fl_renderer_wayland_class_init(FlRendererWaylandClass* klass) {
  FL_RENDERER_CLASS(klass)->set_window = fl_renderer_wayland_set_window;
  FL_RENDERER_CLASS(klass)->create_display = fl_renderer_wayland_create_display;
  FL_RENDERER_CLASS(klass)->create_surface = fl_renderer_wayland_create_surface;
  FL_RENDERER_CLASS(klass)->set_geometry = fl_renderer_wayland_set_geometry;
}

static void fl_renderer_wayland_init(FlRendererWayland* self) {}

FlRendererWayland* fl_renderer_wayland_new() {
  return FL_RENDERER_WAYLAND(
      g_object_new(fl_renderer_wayland_get_type(), nullptr));
}
