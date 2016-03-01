// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GL_ELEMENT_ARRAY_H_
#define VFX_GL_ELEMENT_ARRAY_H_

#include <memory>
#include <vector>

#include "base/macros.h"
#include "base/logging.h"
#include "ui/gl/gl_bindings.h"

namespace vfx {

template<typename Vertex>
class ElementArrayBuffer {
 public:
  ElementArrayBuffer() : vertex_buffer_(0), index_buffer_(0) { }

  ~ElementArrayBuffer() {
    DeleteBuffers();
  }

  ElementArrayBuffer(ElementArrayBuffer&& other)
    : vertices_(std::move(other.vertices_)),
      indices_(std::move(other.indices_)),
      vertex_buffer_(other.vertex_buffer_),
      index_buffer_(other.index_buffer_) {
    other.vertex_buffer_ = 0;
    other.index_buffer_ = 0;
  }

  ElementArrayBuffer& operator=(ElementArrayBuffer&& other) {
    if (this == &other)
      return *this;
    DeleteBuffers();
    vertices_ = std::move(other.vertices_);
    indices_ = std::move(other.indices_);
    vertex_buffer_ = other.vertex_buffer_;
    index_buffer_ = other.index_buffer_;

    other.vertex_buffer_ = 0;
    other.index_buffer_ = 0;
    return *this;
  }

  typedef uint8_t Index;

  void AddVertex(Vertex vertex) {
    vertices_.push_back(std::move(vertex));
  }

  void AddTriangleIndices(Index a, Index b, Index c) {
    indices_.push_back(a);
    indices_.push_back(b);
    indices_.push_back(c);
  }

  void AddQuadIndices(Index a, Index b, Index c, Index d) {
    indices_.push_back(a);
    indices_.push_back(b);
    indices_.push_back(c);

    indices_.push_back(c);
    indices_.push_back(d);
    indices_.push_back(a);
  }

  void AddTriangle(Vertex p1, Vertex p2, Vertex p3) {
    uint16_t base = vertices_.size();

    vertices_.push_back(std::move(p1));
    vertices_.push_back(std::move(p2));
    vertices_.push_back(std::move(p3));

    indices_.push_back(base + 0);
    indices_.push_back(base + 1);
    indices_.push_back(base + 2);
  }

  void AddQuad(Vertex p1, Vertex p2, Vertex p3, Vertex p4) {
    uint16_t base = vertices_.size();

    vertices_.push_back(std::move(p1));
    vertices_.push_back(std::move(p2));
    vertices_.push_back(std::move(p3));
    vertices_.push_back(std::move(p4));

    indices_.push_back(base + 0);
    indices_.push_back(base + 1);
    indices_.push_back(base + 2);

    indices_.push_back(base + 2);
    indices_.push_back(base + 3);
    indices_.push_back(base + 0);
  }

  const Vertex* vertex_data() const { return vertices_.data(); }
  size_t vertex_count() const { return vertices_.size(); }

  const Index* index_data() const { return indices_.data(); }
  size_t index_count() const { return indices_.size(); }

  void BufferData(GLenum usage) {
    if (!vertex_buffer_)
      glGenBuffersARB(1, &vertex_buffer_);
    if (!index_buffer_)
      glGenBuffersARB(1, &index_buffer_);

    GLsizeiptr vertex_size = vertex_count() * sizeof(Vertex);
    GLsizeiptr index_size = index_count() * sizeof(Index);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBufferData(GL_ARRAY_BUFFER, vertex_size, vertices_.data(), usage);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size, indices_.data(), usage);
  }

  bool is_buffered() const { return vertex_buffer_ && index_buffer_; }

  void Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_);
  }

  void Draw() const {
    DCHECK(is_buffered());
    glDrawElements(GL_TRIANGLES, index_count(), GL_UNSIGNED_BYTE, 0);
  }

 private:
  std::vector<Vertex> vertices_;
  std::vector<Index> indices_;

  GLuint vertex_buffer_;
  GLuint index_buffer_;

  void DeleteBuffers() {
    if (vertex_buffer_) {
      glDeleteBuffersARB(1, &vertex_buffer_);
      vertex_buffer_ = 0;
    }

    if (index_buffer_) {
      glDeleteBuffersARB(1, &index_buffer_);
      index_buffer_ = 0;
    }
  }

  DISALLOW_COPY_AND_ASSIGN(ElementArrayBuffer);
};

}  // namespace vfx

#endif  // VFX_GL_ELEMENT_ARRAY_H_
