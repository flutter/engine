// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_REGISTRAR_PRIVATE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_REGISTRAR_PRIVATE_H_

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_engine.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"

G_BEGIN_DECLS

/**
 * fl_texture_registrar_new:
 * @engine: An #FlEngine.
 *
 * Creates a new #FlTextureRegistrar.
 *
 * Returns: a new #FlTextureRegistrar.
 */
FlTextureRegistrar* fl_texture_registrar_new(FlEngine* engine);

/**
 * fl_texture_registrar_populate_texture:
 * @registrar: an #FlTextureRegistrar.
 * @texture_id: an int64_t.
 * @width: a size_t.
 * @height: a size_t.
 * @opengl_texture: an FlutterOpenGLTexture*.
 *
 * Attempts to populate the given |texture| by copying the contents of the
 * texture identified by |texture_id|.
 *
 * Returns true on success.
 */
bool fl_texture_registrar_populate_texture(
    FlTextureRegistrar* registrar,
    int64_t texture_id,
    size_t width,
    size_t height,
    FlutterOpenGLTexture* opengl_texture);

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_REGISTRAR_PRIVATE_H_
