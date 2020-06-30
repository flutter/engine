// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fl_renderer.h"

#include "flutter/shell/platform/embedder/embedder.h"

G_DEFINE_QUARK(fl_renderer_error_quark, fl_renderer_error)

typedef struct {
  EGLDisplay egl_display;
  EGLSurface egl_surface;
  EGLContext egl_context;

  EGLSurface resource_surface;
  EGLContext resource_context;
} FlRendererPrivate;

G_DEFINE_TYPE_WITH_PRIVATE(FlRenderer, fl_renderer, G_TYPE_OBJECT)

// Gets a string representation of the last EGL error.
static const gchar* get_egl_error() {
  EGLint error = eglGetError();
  switch (error) {
    case EGL_SUCCESS:
      return "Success";
    case EGL_NOT_INITIALIZED:
      return "Not Initialized";
    case EGL_BAD_ACCESS:
      return "Bad Access";
    case EGL_BAD_ALLOC:
      return "Bad Allocation";
    case EGL_BAD_ATTRIBUTE:
      return "Bad Attribute";
    case EGL_BAD_CONTEXT:
      return "Bad Context";
    case EGL_BAD_CONFIG:
      return "Bad Configuration";
    case EGL_BAD_CURRENT_SURFACE:
      return "Bad Current Surface";
    case EGL_BAD_DISPLAY:
      return "Bad Display";
    case EGL_BAD_SURFACE:
      return "Bad Surface";
    case EGL_BAD_MATCH:
      return "Bad Match";
    case EGL_BAD_PARAMETER:
      return "Bad Parameter";
    case EGL_BAD_NATIVE_PIXMAP:
      return "Bad Native Pixmap";
    case EGL_BAD_NATIVE_WINDOW:
      return "Bad Native Window";
    case EGL_CONTEXT_LOST:
      return "Context Lost";
    default:
      return "Unknown Error";
  }
}

// Converts an EGL decimal value to a string.
static gchar* egl_decimal_to_string(EGLint value) {
  return g_strdup_printf("%d", value);
}

// Converts an EGL hexadecimal value to a string.
static gchar* egl_hexadecimal_to_string(EGLint value) {
  return g_strdup_printf("0x%x", value);
}

// Converts an EGL enumerated value to a string.
static gchar* egl_enum_to_string(EGLint value) {
  if (value == EGL_FALSE)
    return g_strdup("EGL_FALSE");
  else if (value == EGL_LUMINANCE_BUFFER)
    return g_strdup("EGL_LUMINANCE_BUFFER");
  else if (value == EGL_NONE)
    return g_strdup("EGL_NONE");
  else if (value == EGL_NON_CONFORMANT_CONFIG)
    return g_strdup("EGL_NON_CONFORMANT_CONFIG");
  else if (value == EGL_RGB_BUFFER)
    return g_strdup("EGL_RGB_BUFFER");
  else if (value == EGL_SLOW_CONFIG)
    return g_strdup("EGL_SLOW_CONFIG");
  else if (value == EGL_TRANSPARENT_RGB)
    return g_strdup("EGL_TRANSPARENT_RGB");
  else if (value == EGL_TRUE)
    return g_strdup("EGL_TRUE");
  else
    return nullptr;
}

// Ensures the given bit is not set in a bitfield. Returns TRUE if that bit was
// cleared.
static gboolean clear_bit(EGLint* field, EGLint bit) {
  if ((*field & bit) == 0)
    return FALSE;

  *field ^= bit;
  return TRUE;
}

// Converts an EGL renderable type bitfield to a string.
static gchar* egl_renderable_type_to_string(EGLint value) {
  EGLint v = value;
  g_autoptr(GPtrArray) strings = g_ptr_array_new_with_free_func(g_free);
  if (clear_bit(&v, EGL_OPENGL_ES_BIT))
    g_ptr_array_add(strings, g_strdup("EGL_OPENGL_ES_BIT"));
  if (clear_bit(&v, EGL_OPENVG_BIT))
    g_ptr_array_add(strings, g_strdup("EGL_OPENVG_BIT"));
  if (clear_bit(&v, EGL_OPENGL_ES2_BIT))
    g_ptr_array_add(strings, g_strdup("EGL_OPENGL_ES2_BIT"));
  if (clear_bit(&v, EGL_OPENGL_BIT))
    g_ptr_array_add(strings, g_strdup("EGL_OPENGL_BIT"));
  if (clear_bit(&v, EGL_OPENGL_ES3_BIT))
    g_ptr_array_add(strings, g_strdup("EGL_OPENGL_ES3_BIT"));
  if (v != 0)
    g_ptr_array_add(strings, egl_hexadecimal_to_string(v));
  g_ptr_array_add(strings, nullptr);

  return g_strjoinv("|", reinterpret_cast<gchar**>(strings->pdata));
}

// Converts an EGL surface type bitfield to a string.
static gchar* egl_surface_type_to_string(EGLint value) {
  EGLint v = value;
  g_autoptr(GPtrArray) strings = g_ptr_array_new_with_free_func(g_free);
  if (clear_bit(&v, EGL_PBUFFER_BIT))
    g_ptr_array_add(strings, g_strdup("EGL_PBUFFER_BIT"));
  if (clear_bit(&v, EGL_PIXMAP_BIT))
    g_ptr_array_add(strings, g_strdup("EGL_PIXMAP_BIT"));
  if (clear_bit(&v, EGL_WINDOW_BIT))
    g_ptr_array_add(strings, g_strdup("EGL_WINDOW_BIT"));
  if (v != 0)
    g_ptr_array_add(strings, egl_hexadecimal_to_string(v));
  g_ptr_array_add(strings, nullptr);

  return g_strjoinv("|", reinterpret_cast<gchar**>(strings->pdata));
}

// Generates a string containing information about the given EGL configuration.
static gchar* egl_config_to_string(EGLDisplay display, EGLConfig config) {
  struct {
    EGLint attribute;
    const gchar* name;
    gchar* (*to_string)(EGLint value);
  } config_items[] = {{
                          EGL_CONFIG_ID,
                          "EGL_CONFIG_ID",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_BUFFER_SIZE,
                          "EGL_BUFFER_SIZE",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_COLOR_BUFFER_TYPE,
                          "EGL_COLOR_BUFFER_TYPE",
                          egl_enum_to_string,
                      },
                      {
                          EGL_TRANSPARENT_TYPE,
                          "EGL_TRANSPARENT_TYPE",
                          egl_enum_to_string,
                      },
                      {
                          EGL_LEVEL,
                          "EGL_LEVEL",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_RED_SIZE,
                          "EGL_RED_SIZE",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_GREEN_SIZE,
                          "EGL_GREEN_SIZE",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_BLUE_SIZE,
                          "EGL_BLUE_SIZE",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_ALPHA_SIZE,
                          "EGL_ALPHA_SIZE",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_DEPTH_SIZE,
                          "EGL_DEPTH_SIZE",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_STENCIL_SIZE,
                          "EGL_STENCIL_SIZE",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_SAMPLES,
                          "EGL_SAMPLES",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_SAMPLE_BUFFERS,
                          "EGL_SAMPLE_BUFFERS",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_NATIVE_VISUAL_ID,
                          "EGL_NATIVE_VISUAL_ID",
                          egl_hexadecimal_to_string,
                      },
                      {
                          EGL_NATIVE_VISUAL_TYPE,
                          "EGL_NATIVE_VISUAL_TYPE",
                          egl_hexadecimal_to_string,
                      },
                      {
                          EGL_NATIVE_RENDERABLE,
                          "EGL_NATIVE_RENDERABLE",
                          egl_enum_to_string,
                      },
                      {
                          EGL_CONFIG_CAVEAT,
                          "EGL_CONFIG_CAVEAT",
                          egl_enum_to_string,
                      },
                      {
                          EGL_BIND_TO_TEXTURE_RGB,
                          "EGL_BIND_TO_TEXTURE_RGB",
                          egl_enum_to_string,
                      },
                      {
                          EGL_BIND_TO_TEXTURE_RGBA,
                          "EGL_BIND_TO_TEXTURE_RGBA",
                          egl_enum_to_string,
                      },
                      {
                          EGL_RENDERABLE_TYPE,
                          "EGL_RENDERABLE_TYPE",
                          egl_renderable_type_to_string,
                      },
                      {
                          EGL_CONFORMANT,
                          "EGL_CONFORMANT",
                          egl_renderable_type_to_string,
                      },
                      {
                          EGL_SURFACE_TYPE,
                          "EGL_SURFACE_TYPE",
                          egl_surface_type_to_string,
                      },
                      {
                          EGL_MAX_PBUFFER_WIDTH,
                          "EGL_MAX_PBUFFER_WIDTH",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_MAX_PBUFFER_HEIGHT,
                          "EGL_MAX_PBUFFER_HEIGHT",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_MAX_PBUFFER_PIXELS,
                          "EGL_MAX_PBUFFER_PIXELS",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_MIN_SWAP_INTERVAL,
                          "EGL_MIN_SWAP_INTERVAL",
                          egl_decimal_to_string,
                      },
                      {
                          EGL_MAX_SWAP_INTERVAL,
                          "EGL_MAX_SWAP_INTERVAL",
                          egl_decimal_to_string,
                      },
                      {EGL_NONE, nullptr, nullptr}};

  g_autoptr(GPtrArray) strings = g_ptr_array_new_with_free_func(g_free);
  for (int i = 0; config_items[i].attribute != EGL_NONE; i++) {
    EGLint value;
    if (!eglGetConfigAttrib(display, config, config_items[i].attribute, &value))
      continue;
    g_autofree gchar* value_string = config_items[i].to_string(value);
    if (value_string == nullptr)
      value_string = egl_hexadecimal_to_string(value);
    g_ptr_array_add(
        strings, g_strdup_printf("%s=%s", config_items[i].name, value_string));
  }
  g_ptr_array_add(strings, nullptr);

  return g_strjoinv(" ", reinterpret_cast<gchar**>(strings->pdata));
}

// Creates a resource surface.
static void create_resource_surface(FlRenderer* self, EGLConfig config) {
  FlRendererPrivate* priv =
      static_cast<FlRendererPrivate*>(fl_renderer_get_instance_private(self));

  EGLint context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  const EGLint resource_context_attribs[] = {EGL_WIDTH, 1, EGL_HEIGHT, 1,
                                             EGL_NONE};
  priv->resource_surface = eglCreatePbufferSurface(priv->egl_display, config,
                                                   resource_context_attribs);
  if (priv->resource_surface == nullptr) {
    g_warning("Failed to create EGL resource surface: %s", get_egl_error());
    return;
  }

  priv->resource_context = eglCreateContext(
      priv->egl_display, config, priv->egl_context, context_attributes);
  if (priv->resource_context == nullptr)
    g_warning("Failed to create EGL resource context: %s", get_egl_error());
}

// Default implementation for the start virtual method.
// Provided so subclasses can chain up to here.
static gboolean fl_renderer_real_start(FlRenderer* self, GError** error) {
  FlRendererPrivate* priv =
      static_cast<FlRendererPrivate*>(fl_renderer_get_instance_private(self));

  // Note the use of EGL_DEFAULT_DISPLAY rather than sharing an existing
  // display connection (e.g. an X11 connection from GTK). This is because
  // this EGL display is going to be accessed by a thread from Flutter. In the
  // case of GTK/X11 the display connection is not thread safe and this would
  // cause a crash.
  priv->egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

  if (!eglInitialize(priv->egl_display, nullptr, nullptr)) {
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to initialze EGL");
    return FALSE;
  }

  EGLint attributes[] = {EGL_RENDERABLE_TYPE,
                         EGL_OPENGL_ES2_BIT,
                         EGL_RED_SIZE,
                         8,
                         EGL_GREEN_SIZE,
                         8,
                         EGL_BLUE_SIZE,
                         8,
                         EGL_ALPHA_SIZE,
                         8,
                         EGL_NONE};
  EGLConfig egl_config;
  EGLint n_config;
  if (!eglChooseConfig(priv->egl_display, attributes, &egl_config, 1,
                       &n_config)) {
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to choose EGL config: %s", get_egl_error());
    return FALSE;
  }
  if (n_config == 0) {
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to find appropriate EGL config: %s", get_egl_error());
    return FALSE;
  }
  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    g_autofree gchar* config_string =
        egl_config_to_string(priv->egl_display, egl_config);
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to bind EGL OpenGL ES API using configuration (%s): %s",
                config_string, get_egl_error());
    return FALSE;
  }

  priv->egl_surface = FL_RENDERER_GET_CLASS(self)->create_surface(
      self, priv->egl_display, egl_config);
  if (priv->egl_surface == nullptr) {
    g_autofree gchar* config_string =
        egl_config_to_string(priv->egl_display, egl_config);
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to create EGL surface using configuration (%s): %s",
                config_string, get_egl_error());
    return FALSE;
  }
  EGLint context_attributes[] = {EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE};
  priv->egl_context = eglCreateContext(priv->egl_display, egl_config,
                                       EGL_NO_CONTEXT, context_attributes);
  if (priv->egl_context == nullptr) {
    g_autofree gchar* config_string =
        egl_config_to_string(priv->egl_display, egl_config);
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to create EGL context using configuration (%s): %s",
                config_string, get_egl_error());
    return FALSE;
  }

  create_resource_surface(self, egl_config);

  EGLint value;
  eglQueryContext(priv->egl_display, priv->egl_context,
                  EGL_CONTEXT_CLIENT_VERSION, &value);

  return TRUE;
}

static void fl_renderer_class_init(FlRendererClass* klass) {
  klass->start = fl_renderer_real_start;
}

static void fl_renderer_init(FlRenderer* self) {}

gboolean fl_renderer_start(FlRenderer* self, GError** error) {
  return FL_RENDERER_GET_CLASS(self)->start(self, error);
}

void* fl_renderer_get_proc_address(FlRenderer* self, const char* name) {
  return reinterpret_cast<void*>(eglGetProcAddress(name));
}

gboolean fl_renderer_make_current(FlRenderer* self, GError** error) {
  FlRendererPrivate* priv =
      static_cast<FlRendererPrivate*>(fl_renderer_get_instance_private(self));

  if (!eglMakeCurrent(priv->egl_display, priv->egl_surface, priv->egl_surface,
                      priv->egl_context)) {
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to make EGL context current: %s", get_egl_error());
    return FALSE;
  }

  return TRUE;
}

gboolean fl_renderer_make_resource_current(FlRenderer* self, GError** error) {
  FlRendererPrivate* priv =
      static_cast<FlRendererPrivate*>(fl_renderer_get_instance_private(self));

  if (priv->resource_surface == nullptr || priv->resource_context == nullptr)
    return FALSE;

  if (!eglMakeCurrent(priv->egl_display, priv->resource_surface,
                      priv->resource_surface, priv->resource_context)) {
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to make EGL context current: %s", get_egl_error());
    return FALSE;
  }

  return TRUE;
}

gboolean fl_renderer_clear_current(FlRenderer* self, GError** error) {
  FlRendererPrivate* priv =
      static_cast<FlRendererPrivate*>(fl_renderer_get_instance_private(self));

  if (!eglMakeCurrent(priv->egl_display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                      EGL_NO_CONTEXT)) {
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to clear EGL context: %s", get_egl_error());
    return FALSE;
  }

  return TRUE;
}

guint32 fl_renderer_get_fbo(FlRenderer* self) {
  // There is only one frame buffer object - always return that.
  return 0;
}

gboolean fl_renderer_present(FlRenderer* self, GError** error) {
  FlRendererPrivate* priv =
      static_cast<FlRendererPrivate*>(fl_renderer_get_instance_private(self));

  if (!eglSwapBuffers(priv->egl_display, priv->egl_surface)) {
    g_set_error(error, fl_renderer_error_quark(), FL_RENDERER_ERROR_FAILED,
                "Failed to swap EGL buffers: %s", get_egl_error());
    return FALSE;
  }

  return TRUE;
}
