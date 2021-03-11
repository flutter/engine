// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_REGISTRAR_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_REGISTRAR_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <glib-object.h>
#include <stdint.h>

#include "fl_texture.h"

G_BEGIN_DECLS

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
 * @texture: (transfer-full): a #FlTexture for registration.
 *
 * Registers a texture and returns the unique ID for that texture.
 *
 * Returns: unique ID of the texture.
 */
int64_t fl_texture_registrar_register_texture(FlTextureRegistrar* registrar,
                                              FlTexture* texture);

/**
 * fl_texture_registrar_mark_texture_frame_available:
 * @registrar: an #FlTextureRegistrar.
 * @texture_id: the ID of the texture that has a frame available.
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
 * @texture_id: the ID of the texture that is being unregistered.
 *
 * Unregisters an existing texture object.
 */
void fl_texture_registrar_unregister_texture(FlTextureRegistrar* registrar,
                                             int64_t texture_id);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_REGISTRAR_H_
