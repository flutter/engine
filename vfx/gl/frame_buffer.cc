// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/gl/frame_buffer.h"

#include "ui/gl/gl_bindings.h"

namespace vfx {

FrameBuffer::FrameBuffer() : id_(0) {
}

FrameBuffer::FrameBuffer(gfx::Size size)
  : id_(0), size_(std::move(size)) {
  color_ = Texture::CreateRGBA(size_);
  depth_ = Texture::CreateDepth(size_);

  glGenFramebuffersEXT(1, &id_);
  glBindFramebufferEXT(GL_FRAMEBUFFER, id_);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, color_.id(), 0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_.id(), 0);

  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
  if (status != GL_FRAMEBUFFER_COMPLETE)
    LOG(FATAL) << "Failed to create frame buffer: " << status;
}

FrameBuffer::~FrameBuffer() {
  DeleteFrameBuffer();
}

FrameBuffer::FrameBuffer(FrameBuffer&& other)
  : id_(other.id_),
    size_(std::move(other.size_)),
    color_(std::move(other.color_)),
    depth_(std::move(other.depth_)) {
  other.id_ = 0;
}

FrameBuffer& FrameBuffer::operator=(FrameBuffer&& other) {
  if (this == &other)
    return *this;
  DeleteFrameBuffer();
  id_ = other.id_;
  size_ = std::move(other.size_);
  color_ = std::move(other.color_);
  depth_ = std::move(other.depth_);

  other.id_ = 0;
  return *this;
}

void FrameBuffer::Bind() const {
  glBindFramebufferEXT(GL_FRAMEBUFFER, id_);
}

Texture FrameBuffer::TakeColor() {
  return std::move(color_);
}

Texture FrameBuffer::TakeDepth() {
  return std::move(depth_);
}

void FrameBuffer::DeleteFrameBuffer() {
  if (id_) {
    glDeleteFramebuffersEXT(1, &id_);
    id_ = 0;
  }
}

}  // namespace vfx
