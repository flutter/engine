// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Foundation/Foundation.h>

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterGLTestUtils.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterIOSurfaceHolder.h"

#import <OpenGL/gl.h>
#import "flutter/testing/testing.h"

namespace flutter::testing {

TEST(FlutterIOSurfaceHolder, BindSurface) {
  NSOpenGLContext* context = CreateTestOpenGLContext();

  [context makeCurrentContext];

  GLuint fbo;
  GLuint tex;

  glGenFramebuffers(1, &fbo);
  glGenTextures(1, &tex);

  FlutterIOSurfaceHolder* ioSurfaceHolder = [[FlutterIOSurfaceHolder alloc] init];

  [ioSurfaceHolder bindSurfaceToTexture:tex fbo:fbo size:CGSizeMake(1.0f, 2.0f)];

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  EXPECT_TRUE(status == GL_FRAMEBUFFER_COMPLETE);

  IOSurfaceRef ioSurface = [ioSurfaceHolder ioSurface];
  EXPECT_EQ(IOSurfaceGetWidth(ioSurface), 1u);
  EXPECT_EQ(IOSurfaceGetHeight(ioSurface), 2u);

  [ioSurfaceHolder recreateIOSurfaceWithSize:CGSizeMake(3.0f, 4.0f)];

  EXPECT_EQ(IOSurfaceGetWidth(ioSurface), 3u);
  EXPECT_EQ(IOSurfaceGetHeight(ioSurface), 4u);
}

}  // flutter::testing
