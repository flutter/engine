// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/linux/fl_texture_gl_private.h"
#include "flutter/shell/platform/linux/fl_texture_private.h"
#include "flutter/shell/platform/linux/fl_texture_registrar_private.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture.h"
#include "flutter/shell/platform/linux/public/flutter_linux/fl_texture_registrar.h"
#include "flutter/shell/platform/linux/testing/fl_test.h"
#include "gtest/gtest.h"

#include <epoxy/gl.h>

static constexpr uint32_t BUFFER_WIDTH = 4u;
static constexpr uint32_t BUFFER_HEIGHT = 4u;
static constexpr uint32_t REAL_BUFFER_WIDTH = 2u;
static constexpr uint32_t REAL_BUFFER_HEIGHT = 2u;

G_DECLARE_FINAL_TYPE(FlTestTexture,
                     fl_test_texture,
                     FL,
                     TEST_TEXTURE,
                     FlTextureGL)

/// A simple texture.
struct _FlTestTexture {
  FlTextureGL parent_instance;
};

G_DEFINE_TYPE(FlTestTexture, fl_test_texture, fl_texture_get_type())

static gboolean fl_test_texture_populate(FlTextureGL* texture,
                                         uint32_t* target,
                                         uint32_t* name,
                                         uint32_t* width,
                                         uint32_t* height,
                                         GError** error) {
  EXPECT_TRUE(FL_IS_TEST_TEXTURE(texture));

  EXPECT_EQ(*width, BUFFER_WIDTH);
  EXPECT_EQ(*height, BUFFER_HEIGHT);
  *target = GL_TEXTURE_2D;
  *name = 1;
  *width = REAL_BUFFER_WIDTH;
  *height = REAL_BUFFER_HEIGHT;

  return TRUE;
}

static void fl_test_texture_class_init(FlTestTextureClass* klass) {
  FL_TEXTURE_GL_CLASS(klass)->populate = fl_test_texture_populate;
}

static void fl_test_texture_init(FlTestTexture* self) {}

static FlTestTexture* fl_test_texture_new() {
  return FL_TEST_TEXTURE(g_object_new(fl_test_texture_get_type(), nullptr));
}

// Test that getting the texture ID works.
TEST(FlTextureTest, TextureID) {
  // Texture ID is not assigned until the testure is populated.
  g_autoptr(FlTexture) texture = FL_TEXTURE(fl_test_texture_new());
  EXPECT_EQ(fl_texture_get_texture_id(texture),
            reinterpret_cast<int64_t>(texture));
}

// Test that populating an OpenGL texture works.
TEST(FlTextureTest, PopulateTexture) {
  g_autoptr(FlTextureGL) texture = FL_TEXTURE_GL(fl_test_texture_new());
  FlutterOpenGLTexture opengl_texture = {0};
  g_autoptr(GError) error = nullptr;
  EXPECT_TRUE(fl_texture_gl_populate(texture, BUFFER_WIDTH, BUFFER_HEIGHT,
                                     &opengl_texture, &error));
  EXPECT_EQ(error, nullptr);
  EXPECT_EQ(opengl_texture.width, REAL_BUFFER_WIDTH);
  EXPECT_EQ(opengl_texture.height, REAL_BUFFER_HEIGHT);
}
