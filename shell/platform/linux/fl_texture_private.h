// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_PRIVATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_PRIVATE_H_

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"

G_BEGIN_DECLS

/**
 * fl_texture_populate_texture:
 * @texture: an #FlTexture.
 * @width: width of the texture.
 * @height: height of the texture.
 * @opengl_texture: (out): return an #FlutterOpenGLTexture.
 *
 * Attempts to populate the specified @opengl_texture with texture details
 * such as the name, width, height and the pixel format.
 *
 * Returns: %TRUE on success.
 */
gboolean fl_texture_populate_texture(FlTexture* texture,
                                     uint32_t width,
                                     uint32_t height,
                                     FlutterOpenGLTexture* opengl_texture);

/**
 * fl_texture_get_texture_id:
 * @texture: an #FlTexture.
 *
 * Retrieves the unique id of this texture.
 *
 * Returns: the unique id of this texture.
 */
int64_t fl_texture_get_texture_id(FlTexture* texture);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_PRIVATE_H_
