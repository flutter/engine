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

G_DECLARE_DERIVABLE_TYPE(FlPixelBufferTexture,
                         fl_pixel_buffer_texture,
                         FL,
                         PIXEL_BUFFER_TEXTURE,
                         FlTexture)

struct _FlPixelBufferTextureClass {
  FlTextureClass parent_class;

  /**
   * FlPixelBufferTexture::copy_pixels:
   * @texture: an #FlPixelBufferTexture.
   * @buffer: (out): pixel data.
   * @width: (inout): width of the texture in pixels.
   * @height: (inout): height of the texture in pixels.
   * @error: (allow-none): #GError location to store the error occurring, or
   * %NULL to ignore.
   *
   * Retrieve pixel buffer in RGBA format.
   *
   * As this method is usually invoked from the render thread, you must
   * take care of proper synchronization. It also needs to be ensured that
   * the returned buffer is not released prior to unregistering this texture.
   *
   * Returns: %TRUE on success.
   */
  gboolean (*copy_pixels)(FlPixelBufferTexture* texture,
                          const uint8_t** buffer,
                          uint32_t* width,
                          uint32_t* height,
                          GError** error);
};

/**
 * FlPixelBufferTexture:
 *
 * #FlPixelBufferTexture represents an OpenGL texture generated from a pixel
 * buffer.
 */

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_PIXEL_BUFFER_TEXTURE_H_
