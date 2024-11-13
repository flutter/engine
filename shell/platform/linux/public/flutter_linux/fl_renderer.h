// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_PUBLIC_FLUTTER_LINUX_FL_RENDERER_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_PUBLIC_FLUTTER_LINUX_FL_RENDERER_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <glib-object.h>
#include <gmodule.h>

#ifdef FLUTTER_HAS_EMBEDDER_API
#include <flutter_embedder.h>
#endif

#include "fl_engine.h"

G_BEGIN_DECLS

G_MODULE_EXPORT
G_DECLARE_DERIVABLE_TYPE(FlRenderer, fl_renderer, FL, RENDERER, GObject)

/**
 * FlRenderer:
 *
 * #FlRenderer is an abstract class that allows Flutter to draw pixels.
 */

struct _FlRendererClass {
  GObjectClass parent_class;

  /**
   * Virtual method called when Flutter needs to make the OpenGL context
   * current.
   * @renderer: an #FlRenderer.
   */
  void (*make_current)(FlRenderer* renderer);

  /**
   * Virtual method called when Flutter needs to make the OpenGL resource
   * context current.
   * @renderer: an #FlRenderer.
   */
  void (*make_resource_current)(FlRenderer* renderer);

  /**
   * Virtual method called when Flutter needs to clear the OpenGL context.
   * @renderer: an #FlRenderer.
   */
  void (*clear_current)(FlRenderer* renderer);

  /**
   * Virtual method called when Flutter wants to get the refresh rate of the
   * renderer.
   * @renderer: an #FlRenderer.
   *
   * Returns: The refresh rate of the display in Hz. If the refresh rate is
   * not available, returns -1.0.
   */
  gdouble (*get_refresh_rate)(FlRenderer* renderer);
};

/**
 * fl_renderer_make_current:
 * @renderer: an #FlRenderer.
 *
 * Makes the rendering context current.
 */
void fl_renderer_make_current(FlRenderer* renderer);

/**
 * fl_renderer_make_resource_current:
 * @renderer: an #FlRenderer.
 *
 * Makes the resource rendering context current.
 */
void fl_renderer_make_resource_current(FlRenderer* renderer);

/**
 * fl_renderer_clear_current:
 * @renderer: an #FlRenderer.
 *
 * Clears the current rendering context.
 */
void fl_renderer_clear_current(FlRenderer* renderer);

/**
 * fl_renderer_get_fbo:
 * @renderer: an #FlRenderer.
 *
 * Gets the frame buffer object to render to.
 *
 * Returns: a frame buffer object index.
 */
guint32 fl_renderer_get_fbo(FlRenderer* renderer);

/**
 * fl_renderer_get_refresh_rate:
 * @renderer: an #FlRenderer.
 *
 * Returns: The refresh rate of the display in Hz. If the refresh rate is
 * not available, returns -1.0.
 */
gdouble fl_renderer_get_refresh_rate(FlRenderer* renderer);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_PUBLIC_FLUTTER_LINUX_FL_RENDERER_H_
