// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GL_TEXTURE_H_
#define VFX_GL_TEXTURE_H_

#include <GL/gl.h>

#include "base/macros.h"
#include "ui/gfx/geometry/size.h"

namespace vfx {

class Texture {
 public:
  struct Desc {
    Desc();

    gfx::Size size;
    GLint internal_format;
    GLenum format;
    GLenum type;
  };

  Texture();
  explicit Texture(const Desc& desc);
  ~Texture();

  static Texture CreateRGBA(gfx::Size size);
  static Texture CreateDepth(gfx::Size size);

  Texture(Texture&& other);
  Texture& operator=(Texture&& other);

  bool is_null() const { return id_ == 0; }
  GLuint id() const { return id_; }

  void Bind() const;

 private:
  GLuint id_;
  gfx::Size size_;

  void DeleteTexture();

  DISALLOW_COPY_AND_ASSIGN(Texture);
};

}  // namespace vfx

#endif  // VFX_GL_TEXTURE_H_
