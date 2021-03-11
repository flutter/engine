// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture.h"
#include "flutter/shell/platform/linux/fl_texture_private.h"
#include "flutter/shell/platform/linux/fl_texture_registrar_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"
#include "gtest/gtest.h"

#include <epoxy/gl.h>

// Test that getting the texture ID works.
TEST(FlTextureTest, TextureID) {
  // Texture ID is not assigned until the pixel buffer is copied once.
  static constexpr uint32_t BUFFER_WIDTH = 4u;
  static constexpr uint32_t BUFFER_HEIGHT = 4u;
  static constexpr uint32_t REAL_BUFFER_WIDTH = 2u;
  static constexpr uint32_t REAL_BUFFER_HEIGHT = 2u;
  FlCopyPixelBufferCallback callback =
      [](const uint8_t** out_buffer, uint32_t* format, uint32_t* width,
         uint32_t* height, gpointer user_data) -> gboolean {
    static const uint8_t buffer[] = {0xc9, 0xc8, 0xc7, 0xc6};
    EXPECT_EQ(*width, BUFFER_WIDTH);
    EXPECT_EQ(*height, BUFFER_HEIGHT);
    *out_buffer = buffer;
    *format = GL_R8;
    *width = REAL_BUFFER_WIDTH;
    *height = REAL_BUFFER_HEIGHT;
    return TRUE;
  };
  g_autoptr(FlTexture) texture =
      FL_TEXTURE(fl_pixel_buffer_texture_new(callback, nullptr, nullptr));
  EXPECT_EQ(fl_texture_get_texture_id(texture),
            reinterpret_cast<int64_t>(texture));
}

// Test that populating an OpenGL texture works.
TEST(FlTextureTest, PopulateTexture) {
  static constexpr uint32_t BUFFER_WIDTH = 4u;
  static constexpr uint32_t BUFFER_HEIGHT = 4u;
  static constexpr uint32_t REAL_BUFFER_WIDTH = 2u;
  static constexpr uint32_t REAL_BUFFER_HEIGHT = 2u;
  FlCopyPixelBufferCallback callback =
      [](const uint8_t** out_buffer, uint32_t* format, uint32_t* width,
         uint32_t* height, gpointer user_data) -> gboolean {
    static const uint8_t buffer[] = {0x7a, 0x8a, 0x9a, 0xaa};
    EXPECT_EQ(*width, BUFFER_WIDTH);
    EXPECT_EQ(*height, BUFFER_HEIGHT);
    *out_buffer = buffer;
    *format = GL_R8;
    *width = REAL_BUFFER_WIDTH;
    *height = REAL_BUFFER_HEIGHT;
    return TRUE;
  };
  g_autoptr(FlTexture) texture =
      FL_TEXTURE(fl_pixel_buffer_texture_new(callback, nullptr, nullptr));
  FlutterOpenGLTexture opengl_texture = {0};
  EXPECT_TRUE(fl_texture_populate_texture(texture, BUFFER_WIDTH, BUFFER_HEIGHT,
                                          &opengl_texture));
  EXPECT_EQ(opengl_texture.width, REAL_BUFFER_WIDTH);
  EXPECT_EQ(opengl_texture.height, REAL_BUFFER_HEIGHT);
}
