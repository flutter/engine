// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"
#include "flutter/shell/platform/linux/fl_texture_registrar_private.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"
#include "gtest/gtest.h"

#include <epoxy/gl.h>

#include <gmodule.h>

// Test that registering a texture works.
TEST(FlTextureRegistrarTest, RegisterTexture) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlTextureRegistrar) registrar = fl_texture_registrar_new(engine);
  FlTexture* texture =
      FL_TEXTURE(fl_pixel_buffer_texture_new(nullptr, nullptr, nullptr));
  int64_t id = fl_texture_registrar_register_texture(registrar, texture);

  EXPECT_EQ(fl_texture_registrar_get_texture(registrar, id), texture);

  fl_texture_registrar_unregister_texture(registrar, id);

  EXPECT_EQ(fl_texture_registrar_get_texture(registrar, id), nullptr);
}

// Test that marking a texture frame available works.
TEST(FlTextureRegistrarTest, MarkTextureFrameAvailable) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlTextureRegistrar) registrar = fl_texture_registrar_new(engine);
  int64_t id = fl_texture_registrar_register_texture(registrar, nullptr);
  fl_texture_registrar_mark_texture_frame_available(registrar, id);
}

// Test that populating an OpenGL texture works.
TEST(FlTextureRegistrarTest, PopulateTexture) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  g_autoptr(FlTextureRegistrar) registrar = fl_texture_registrar_new(engine);
  static constexpr uint32_t BUFFER_WIDTH = 4u;
  static constexpr uint32_t BUFFER_HEIGHT = 4u;
  static constexpr uint32_t REAL_BUFFER_WIDTH = 2u;
  static constexpr uint32_t REAL_BUFFER_HEIGHT = 2u;
  FlCopyPixelBufferCallback callback =
      [](const uint8_t** out_buffer, uint32_t* format, uint32_t* width,
         uint32_t* height, gpointer user_data) -> gboolean {
    const uint8_t buffer[] = {0x7a, 0x8a, 0x9a, 0xaa};
    EXPECT_EQ(*width, BUFFER_WIDTH);
    EXPECT_EQ(*height, BUFFER_HEIGHT);
    *out_buffer = buffer;
    *format = GL_R8;
    *width = REAL_BUFFER_WIDTH;
    *height = REAL_BUFFER_HEIGHT;
    return TRUE;
  };
  FlTexture* texture =
      FL_TEXTURE(fl_pixel_buffer_texture_new(callback, nullptr, nullptr));
  int64_t id = fl_texture_registrar_register_texture(registrar, texture);
  FlutterOpenGLTexture opengl_texture;
  EXPECT_TRUE(fl_texture_registrar_populate_texture(
      registrar, id, BUFFER_WIDTH, BUFFER_HEIGHT, &opengl_texture));
  EXPECT_EQ(opengl_texture.width, REAL_BUFFER_WIDTH);
  EXPECT_EQ(opengl_texture.height, REAL_BUFFER_HEIGHT);
}
