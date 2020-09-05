// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"
#include "flutter/shell/platform/linux/fl_texture_registrar_private.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"
#include "gtest/gtest.h"

#include <gmodule.h>

// Test that registering a texture works.
TEST(FlTextureRegistrarTest, RegisterTexture) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  FlTextureRegistrar* registrar = fl_texture_registrar_new(engine);
  fl_texture_registrar_register_texture(registrar, nullptr, nullptr);
}

// Test that unregistering a texture works.
TEST(FlTextureRegistrarTest, UnregisterTexture) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  FlTextureRegistrar* registrar = fl_texture_registrar_new(engine);
  int64_t id =
      fl_texture_registrar_register_texture(registrar, nullptr, nullptr);
  fl_texture_registrar_unregister_texture(registrar, id);
}

// Test that marking a texture frame available works.
TEST(FlTextureRegistrarTest, MarkTextureFrameAvailable) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  FlTextureRegistrar* registrar = fl_texture_registrar_new(engine);
  int64_t id =
      fl_texture_registrar_register_texture(registrar, nullptr, nullptr);
  fl_texture_registrar_mark_texture_frame_available(registrar, id);
}

// Test that populating an OpenGL texture works.
TEST(FlTextureRegistrarTest, PopulateTexture) {
  g_autoptr(FlEngine) engine = make_mock_engine();
  FlTextureRegistrar* registrar = fl_texture_registrar_new(engine);
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
  int64_t id =
      fl_texture_registrar_register_texture(registrar, callback, nullptr);
  FlutterOpenGLTexture opengl_texture;
  EXPECT_EQ(fl_texture_registrar_populate_texture(
                registrar, id, BUFFER_WIDTH, BUFFER_HEIGHT, &opengl_texture),
            TRUE);
  EXPECT_EQ(opengl_texture.width, REAL_BUFFER_WIDTH);
  EXPECT_EQ(opengl_texture.height, REAL_BUFFER_HEIGHT);
#undef REAL_BUFFER_HEIGHT
#undef REAL_BUFFER_WIDTH
#undef BUFFER_HEIGHT
#undef BUFFER_WIDTH
}
