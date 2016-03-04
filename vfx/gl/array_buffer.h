// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GEOMETRY_TRIANGLE_STRIP_H_
#define VFX_GEOMETRY_TRIANGLE_STRIP_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "ui/gl/gl_bindings.h"
#include "vfx/geometry/point.h"

namespace vfx {

template<typename Vertex>
class ArrayBuffer {
 public:
  ArrayBuffer() : mode_(GL_TRIANGLE_STRIP), vertex_buffer_(0) { }
  ~ArrayBuffer() {
    if (vertex_buffer_)
      glDeleteBuffersARB(1, &vertex_buffer_);
  }

  ArrayBuffer(GLenum mode, std::vector<Vertex> data)
    : mode_(mode),
      vertices_(std::move(data)),
      vertex_buffer_(0) { }

  ArrayBuffer(ArrayBuffer&& other)
    : mode_(other.mode_),
      vertices_(std::move(other.vertices_)),
      vertex_buffer_(other.vertex_buffer_) {
    other.vertex_buffer_ = 0;
  }

  ArrayBuffer& operator=(ArrayBuffer&& other) {
    if (this == &other)
      return *this;
    mode_ = other.mode_;
    vertices_ = std::move(other.vertices_);
    vertex_buffer_ = other.vertex_buffer_;
    other.vertex_buffer_ = 0;
    return *this;
  }

  const Vertex* data() const { return vertices_.data(); }
  size_t count() const { return vertices_.size(); }

  void BufferData(GLenum usage) {
    if (!vertex_buffer_)
      glGenBuffersARB(1, &vertex_buffer_);
    GLsizeiptr size = vertices_.size() * sizeof(Vertex);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBufferData(GL_ARRAY_BUFFER, size, vertices_.data(), usage);
  }

  bool is_buffered() const { return vertex_buffer_; }

  void Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
  }

  void Draw() const {
    DCHECK(is_buffered());
    glDrawArrays(mode_, 0, vertices_.size());
  }

 private:
  GLenum mode_;
  std::vector<Vertex> vertices_;
  GLuint vertex_buffer_;

  DISALLOW_COPY_AND_ASSIGN(ArrayBuffer);
};

}  // namespace vfx

#endif  // VFX_GEOMETRY_TRIANGLE_STRIP_H_
