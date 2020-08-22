// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_REGISTRAR_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <glib-object.h>
#include <stdint.h>

G_BEGIN_DECLS

/**
 * An image buffer object.
 */
typedef struct {
  const uint8_t* buffer;
  size_t width;
  size_t height;
} FlPixelBuffer;

/**
 * The pixel buffer copy callback provided to the Flutter engine to copy the
 * texture.
 */
typedef const FlPixelBuffer* (*FlTextureCallback)(size_t width,
                                                  size_t height,
                                                  void* user_data);

G_DECLARE_FINAL_TYPE(FlTextureRegistrar,
                     fl_texture_registrar,
                     FL,
                     TEXTURE_REGISTRAR,
                     GObject)

/**
 * FlTextureRegistrar:
 *
 * #FlTextureRegistrar is used when registering textures.
 */

/**
 * fl_texture_registrar_register_texture:
 * @registrar: an #FlTextureRegistrar.
 * @texture: an #FlTexture.
 *
 * Registers a texture callback and returns the ID for that texture.
 *
 * Returns: an int64_t.
 */
int64_t fl_texture_registrar_register_texture(
    FlTextureRegistrar* registrar,
    FlTextureCallback texture_callback,
    void* user_data);

/**
 * fl_texture_registrar_mark_texture_frame_available:
 * @registrar: an #FlTextureRegistrar.
 * @texture_id: an int64_t.
 *
 * Notifies the flutter engine that the texture object corresponding
 * to texture_id needs to render a new texture.
 */
void fl_texture_registrar_mark_texture_frame_available(
    FlTextureRegistrar* registrar,
    int64_t texture_id);

/**
 * fl_texture_registrar_unregister_texture:
 * @registrar: an #FlTextureRegistrar.
 * @texture_id: an int64_t.
 *
 * Unregisters an existing texture object.
 */
void fl_texture_registrar_unregister_texture(FlTextureRegistrar* registrar,
                                             int64_t texture_id);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_REGISTRAR_H_
