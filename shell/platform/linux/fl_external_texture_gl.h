// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_EXTERNAL_TEXURE_GL_H
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_EXTERNAL_TEXURE_GL_H

#include <glib-object.h>
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlExternalTextureGl,
                     fl_external_texture_gl,
                     FL,
                     EXTERNAL_TEXTURE_GL,
                     GObject)

/**
 * FlExternalTextureGl:
 *
 * #FlExternalTextureGl is an abstraction over OpenGL textures.
 */

/**
 * fl_external_texture_gl_new:
 * @texture_callback: An #FlTextureCallback.
 * @user_data: A void*.
 *
 * Creates a new #FlExternalTextureGl.
 *
 * Returns: a new #FlExternalTextureGl.
 */
FlExternalTextureGl* fl_external_texture_gl_new(
    FlTextureCallback texture_callback,
    void* user_data);

/**
 * fl_external_texture_gl_populate_texture:
 * @width: a size_t.
 * @height: a size_t.
 * @opengl_texture: a FlutterOpenGLTexture*.
 *
 * Attempts to populate the specified |opengl_texture| with texture details
 * such as the name, width, height and the pixel format upon successfully
 * copying the buffer provided by |texture_callback_|. See
 * |fl_external_texture_gl_copy_pixel_buffer|.
 *
 * Returns true on success or false if the pixel buffer could not be copied.
 */
bool fl_external_texture_gl_populate_texture(
    FlExternalTextureGl* self,
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture);

/**
 * fl_external_texture_gl_texture_id:
 *
 * Retrieves the unique id of this texture.
 *
 * Returns an int64_t, which is the unique id of this texture.
 */
int64_t fl_external_texture_gl_texture_id(FlExternalTextureGl* self);

/**
 * fl_external_texture_gl_copy_pixel_buffer:
 * @width: a size_t.
 * @height: a size_t.
 *
 * Attempts to copy the pixel buffer returned by |texture_callback_| to
 * OpenGL. The |width| and |height| will be set to the actual bounds of the
 * copied pixel buffer.
 *
 * Returns true on success or false if the pixel buffer returned by
 * |texture_callback_| was invalid.
 */
bool fl_external_texture_gl_copy_pixel_buffer(FlExternalTextureGl* self,
                                              size_t* width,
                                              size_t* height);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_EXTERNAL_TEXURE_GL_H
