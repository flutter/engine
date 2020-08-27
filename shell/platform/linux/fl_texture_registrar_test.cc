// Copyright 2020 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/linux/fl_external_texture_gl.h"
#include "flutter/shell/platform/linux/fl_texture_registrar_private.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"
#include "gtest/gtest.h"

#include <gmodule.h>

// Implements eglGetProcAddress but returns dummy functions.
// static void* eglGetProcAddress(const char* name) {
//   return reinterpret_cast<void*>(+[]() {});
// }

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

// TODO(anirudhb): Re-enable after figuring out how to stub out
// eglGetProcAddress. Also write tests for #FlExternalTextureGl as well.
//
// Test that populating an OpenGL texture works.
// TEST(FlTextureRegistrarTest, PopulateTexture) {
//   g_autoptr(FlEngine) engine = make_mock_engine();
//   FlTextureRegistrar* registrar = fl_texture_registrar_new(engine);
//   const uint8_t buffer[] = {0x7a, 0x8a, 0x9a, 0xaa};
//   FlPixelBuffer pixel_buffer;
//   pixel_buffer.buffer = buffer;
//   pixel_buffer.width = 2;
//   pixel_buffer.height = 2;
//   FlTextureCallback callback = [](size_t width, size_t height,
//                                   void* user_data) -> const FlPixelBuffer* {
//     FlPixelBuffer* pixel_buffer = static_cast<FlPixelBuffer*>(user_data);
//     EXPECT_EQ(width, pixel_buffer->width);
//     EXPECT_EQ(height, pixel_buffer->height);
//     return pixel_buffer;
//   };
//   int64_t id =
//       fl_texture_registrar_register_texture(registrar, callback,
//       &pixel_buffer);
//   FlutterOpenGLTexture opengl_texture;
//   EXPECT_TRUE(fl_texture_registrar_populate_texture(
//       registrar, id, pixel_buffer.width, pixel_buffer.height,
//       &opengl_texture));
//   EXPECT_EQ(opengl_texture.width, pixel_buffer.width);
//   EXPECT_EQ(opengl_texture.height, pixel_buffer.height);
// }
