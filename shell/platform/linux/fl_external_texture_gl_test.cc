// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GL/gl.h>

#include "flutter/shell/platform/linux/fl_external_texture_gl.h"
#include "flutter/shell/platform/linux/fl_texture_registrar_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"
#include "gtest/gtest.h"

#include <gmodule.h>

// Test that getting the texture ID works.
TEST(FlExternalTextureGlTest, TextureID) {
  // Texture ID is not assigned until the pixel buffer is copied once.
#define BUFFER_WIDTH 19u
#define BUFFER_HEIGHT 19u
#define REAL_BUFFER_WIDTH 7u
#define REAL_BUFFER_HEIGHT 7u
  FlTextureCallback callback = [](size_t* width, size_t* height,
                                  const uint8_t** out_buffer,
                                  void* user_data) -> gboolean {
    const uint8_t buffer[] = {0xc9, 0xc8, 0xc7, 0xc6};
    EXPECT_EQ(*width, BUFFER_WIDTH);
    EXPECT_EQ(*height, BUFFER_HEIGHT);
    *out_buffer = buffer;
    *width = REAL_BUFFER_WIDTH;
    *height = REAL_BUFFER_HEIGHT;
    return TRUE;
  };
  FlExternalTextureGl* texture = fl_external_texture_gl_new(callback, nullptr);
  size_t width = BUFFER_WIDTH;
  size_t height = BUFFER_HEIGHT;
  EXPECT_EQ(fl_external_texture_gl_copy_pixel_buffer(texture, &width, &height),
            TRUE);
  EXPECT_EQ(fl_external_texture_gl_texture_id(texture),
            reinterpret_cast<int64_t>(texture));
#undef REAL_BUFFER_HEIGHT
#undef REAL_BUFFER_WIDTH
#undef BUFFER_HEIGHT
#undef BUFFER_WIDTH
}

// Test that copying a pixel buffer works.
TEST(FlExternalTextureGlTest, CopyPixelBuffer) {
#define BUFFER_WIDTH 8u
#define BUFFER_HEIGHT 8u
#define REAL_BUFFER_WIDTH 3u
#define REAL_BUFFER_HEIGHT 3u
  FlTextureCallback callback = [](size_t* width, size_t* height,
                                  const uint8_t** out_buffer,
                                  void* user_data) -> gboolean {
    const uint8_t buffer[] = {0xb3, 0xb4, 0xb5, 0xb6};
    EXPECT_EQ(*width, BUFFER_WIDTH);
    EXPECT_EQ(*height, BUFFER_HEIGHT);
    *out_buffer = buffer;
    *width = REAL_BUFFER_WIDTH;
    *height = REAL_BUFFER_HEIGHT;
    return TRUE;
  };
  FlExternalTextureGl* texture = fl_external_texture_gl_new(callback, nullptr);
  size_t width = BUFFER_WIDTH;
  size_t height = BUFFER_HEIGHT;
  EXPECT_EQ(fl_external_texture_gl_copy_pixel_buffer(texture, &width, &height),
            TRUE);
  EXPECT_EQ(width, REAL_BUFFER_WIDTH);
  EXPECT_EQ(height, REAL_BUFFER_HEIGHT);
#undef REAL_BUFFER_HEIGHT
#undef REAL_BUFFER_WIDTH
#undef BUFFER_HEIGHT
#undef BUFFER_WIDTH
}

// Test that populating an OpenGL texture works.
TEST(FlExternalTextureGlTest, PopulateTexture) {
#define BUFFER_WIDTH 4u
#define BUFFER_HEIGHT 4u
#define REAL_BUFFER_WIDTH 2u
#define REAL_BUFFER_HEIGHT 2u
  FlTextureCallback callback = [](size_t* width, size_t* height,
                                  const uint8_t** out_buffer,
                                  void* user_data) -> gboolean {
    const uint8_t buffer[] = {0x7a, 0x8a, 0x9a, 0xaa};
    EXPECT_EQ(*width, BUFFER_WIDTH);
    EXPECT_EQ(*height, BUFFER_HEIGHT);
    *out_buffer = buffer;
    *width = REAL_BUFFER_WIDTH;
    *height = REAL_BUFFER_HEIGHT;
    return TRUE;
  };
  FlExternalTextureGl* texture = fl_external_texture_gl_new(callback, nullptr);
  FlutterOpenGLTexture opengl_texture;
  EXPECT_EQ(fl_external_texture_gl_populate_texture(
                texture, BUFFER_WIDTH, BUFFER_HEIGHT, &opengl_texture),
            TRUE);
  EXPECT_EQ(opengl_texture.width, REAL_BUFFER_WIDTH);
  EXPECT_EQ(opengl_texture.height, REAL_BUFFER_HEIGHT);
#undef REAL_BUFFER_HEIGHT
#undef REAL_BUFFER_WIDTH
#undef BUFFER_HEIGHT
#undef BUFFER_WIDTH
}
