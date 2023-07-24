// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_EXTERNAL_TEXTURE_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_EXTERNAL_TEXTURE_H_

#include "flutter/shell/platform/embedder/embedder.h"

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

namespace flutter {

typedef void (*glGenFramebuffersProc)(GLsizei n, GLuint* framebuffers);
typedef void (*glDeleteFramebuffersProc)(GLsizei n, const GLuint* framebuffers);
typedef void (*glBindFramebufferProc)(GLenum target, GLuint framebuffer);
typedef void (*glFramebufferTexture2DProc)(GLenum target,
                                           GLenum attachment,
                                           GLenum textarget,
                                           GLuint texture,
                                           GLint level);
typedef void (*glBlitFramebufferANGLEProc)(GLint srcX0,
                                           GLint srcY0,
                                           GLint srcX1,
                                           GLint srcY1,
                                           GLint dstX0,
                                           GLint dstY0,
                                           GLint dstX1,
                                           GLint dstY1,
                                           GLbitfield mask,
                                           GLenum filter);

typedef void (*glGenTexturesProc)(GLsizei n, GLuint* textures);
typedef void (*glDeleteTexturesProc)(GLsizei n, const GLuint* textures);
typedef void (*glBindTextureProc)(GLenum target, GLuint texture);
typedef void (*glTexParameterfProc)(GLenum target, GLenum pname, GLfloat param);
typedef void (*glTexParameteriProc)(GLenum target, GLenum pname, GLint param);
typedef void (*glTexImage2DProc)(GLenum target,
                                 GLint level,
                                 GLint internalformat,
                                 GLsizei width,
                                 GLsizei height,
                                 GLint border,
                                 GLenum format,
                                 GLenum type,
                                 const void* data);

// A struct containing pointers to resolved gl* functions.
struct GlProcs {
  glGenFramebuffersProc glGenFramebuffers;
  glDeleteFramebuffersProc glDeleteFramebuffers;
  glBindFramebufferProc glBindFramebuffer;
  glFramebufferTexture2DProc glFramebufferTexture2D;
  glBlitFramebufferANGLEProc glBlitFramebufferANGLE;

  glGenTexturesProc glGenTextures;
  glDeleteTexturesProc glDeleteTextures;
  glBindTextureProc glBindTexture;
  glTexParameterfProc glTexParameterf;
  glTexParameteriProc glTexParameteri;
  glTexImage2DProc glTexImage2D;
  bool valid;
};

// Abstract external texture.
class ExternalTexture {
 public:
  virtual ~ExternalTexture() = default;

  // Returns the unique id of this texture.
  int64_t texture_id() const { return reinterpret_cast<int64_t>(this); };

  // Attempts to populate the specified |opengl_texture| with texture details
  // such as the name, width, height and the pixel format.
  // Returns true on success.
  virtual bool PopulateTexture(size_t width,
                               size_t height,
                               FlutterOpenGLTexture* opengl_texture) = 0;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_EXTERNAL_TEXTURE_H_
