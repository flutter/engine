// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_H_
#define FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_H_

#if !defined(__FLUTTER_LINUX_INSIDE__) && !defined(FLUTTER_LINUX_COMPILATION)
#error "Only <flutter_linux/flutter_linux.h> can be included directly."
#endif

#include <glib-object.h>
#include <stdint.h>

G_BEGIN_DECLS

G_DECLARE_DERIVABLE_TYPE(FlTexture, fl_texture, FL, TEXTURE, GObject)

/**
 * FlTexture:
 *
 * #FlTexture is an abstract class that represents a texture.
 *
 * You can derive #FlTextureGL for populating hardware-accelerated textures or
 * instantiate #FlPixelBufferTexture for populating pixel buffers. Do NOT
 * directly derive this class.
 */

struct _FlTextureClass {
  GObjectClass parent_class;
};

G_END_DECLS

#endif  // FLUTTER_SHELL_PLATFORM_LINUX_FL_TEXTURE_H_
