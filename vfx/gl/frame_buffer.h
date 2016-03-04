// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GL_FRAME_BUFFER_H_
#define VFX_GL_FRAME_BUFFER_H_

#include <GL/gl.h>

#include "base/macros.h"
#include "ui/gfx/geometry/size.h"
#include "vfx/gl/texture.h"

namespace vfx {

class FrameBuffer {
 public:
  FrameBuffer();
  explicit FrameBuffer(gfx::Size size);
  ~FrameBuffer();

  FrameBuffer(FrameBuffer&& other);
  FrameBuffer& operator=(FrameBuffer&& other);

  void Bind() const;
  Texture TakeColor();
  Texture TakeDepth();

  bool is_null() const { return id_ == 0; }
  GLuint id() const { return id_; }

  const Texture& color() const { return color_; }
  const Texture& depth() const { return depth_; }

 private:
  GLuint id_;
  gfx::Size size_;
  Texture color_;
  Texture depth_;

  void DeleteFrameBuffer();

  DISALLOW_COPY_AND_ASSIGN(FrameBuffer);
};

}  // namespace vfx

#endif  // VFX_GL_FRAME_BUFFER_H_
