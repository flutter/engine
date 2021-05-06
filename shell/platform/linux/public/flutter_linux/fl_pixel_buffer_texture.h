// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_PIXEL_BUFFER_TEXTURE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_PIXEL_BUFFER_TEXTURE_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include "fl_texture.h"

G_BEGIN_DECLS

G_DECLARE_FINAL_TYPE(FlPixelBufferTexture,
                     fl_pixel_buffer_texture,
                     FL,
                     PIXEL_BUFFER_TEXTURE,
                     FlTexture)

/**
 * FlPixelBufferTexture:
 *
 * #FlPixelBufferTexture represents a pixel buffer texture.
 */

/**
 * FlCopyPixelBufferCallback:
 * @buffer: (out): return callee-allocated pixel buffer.
 * @format: (out): return pixel buffer format (example GL_RGBA).
 * @width: (inout): pointer to width of the texture.
 * @height: (inout): pointer to height of the texture.
 * @user_data: (closure): data provided when populating texture at first time.
 *
 * Function called when Flutter wants to copy pixel buffer for populating
 * texture.
 *
 * Returns: %TRUE on success.
 */
typedef gboolean (*FlCopyPixelBufferCallback)(const uint8_t** buffer,
                                              uint32_t* format,
                                              uint32_t* width,
                                              uint32_t* height,
                                              gpointer user_data);

/**
 * fl_pixel_buffer_texture_new:
 * @callback: a callback to copy pixel buffer when populating texture.
 * @user_data: (closure): user data to pass to @callback.
 * @destroy_notify: (allow-none): a function which gets called to free
 * @user_data, or %NULL.
 *
 * Creates a new #FlPixelBufferTexture.
 *
 * Returns: the newly created #FlPixelBufferTexture.
 */
FlPixelBufferTexture* fl_pixel_buffer_texture_new(
    FlCopyPixelBufferCallback callback,
    gpointer user_data,
    GDestroyNotify destroy_notify);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_PIXEL_BUFFER_TEXTURE_H_
