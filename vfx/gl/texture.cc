// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vfx/gl/texture.h"

#include "ui/gl/gl_bindings.h"

namespace vfx {

Texture::Desc::Desc()
  : internal_format(GL_RGBA),
    format(GL_RGBA),
    type(GL_UNSIGNED_BYTE) { 
}

Texture::Texture() : id_(0) {
}

Texture::Texture(const Desc& desc) : id_(0), size_(desc.size) {
  glGenTextures(1, &id_);
  glBindTexture(GL_TEXTURE_2D, id_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, desc.internal_format, desc.size.width(),
               desc.size.height(), 0, desc.format, desc.type, nullptr);
}

Texture::~Texture() {
  DeleteTexture();
}

Texture Texture::CreateRGBA(gfx::Size size) {
  Desc desc;
  desc.size = std::move(size);
  desc.internal_format = GL_RGBA;
  desc.format = GL_RGBA;
  desc.type = GL_UNSIGNED_BYTE;
  return Texture(desc);
}

Texture Texture::CreateDepth(gfx::Size size) {
  Desc desc;
  desc.size = std::move(size);
  desc.internal_format = GL_DEPTH_COMPONENT;
  desc.format = GL_DEPTH_COMPONENT;
  desc.type = GL_UNSIGNED_INT;
  return Texture(desc);
}

Texture::Texture(Texture&& other)
  : id_(other.id_),
    size_(std::move(other.size_)) {
  other.id_ = 0;
}

Texture& Texture::operator=(Texture&& other) {
  if (this == &other)
    return *this;
  DeleteTexture();
  id_ = other.id_;
  size_ = std::move(other.size_);

  other.id_ = 0;
  return *this;
}

void Texture::Bind() const {
  glBindTexture(GL_TEXTURE_2D, id_);
}

void Texture::DeleteTexture() {
  if (id_) {
    glDeleteTextures(1, &id_);
    id_ = 0;
  }
}

}  // namespace vfx
