// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GEOMETRY_CUBOID_H_
#define VFX_GEOMETRY_CUBOID_H_

#include <vector>

#include "vfx/geometry/point.h"
#include "vfx/geometry/quad.h"

namespace vfx {

// A solid bounded by six planes.
class Cuboid {
 public:
  Cuboid() { }
  Cuboid(const Quad& a, const Quad& b) {
    a_ = a;
    b_ = b;
  }

  const Quad& a() const { return a_; }
  const Quad& b() const { return b_; }

  void set_a(const Quad& a) { a_ = a; }
  void set_b(const Quad& b) { b_ = b; }

  template<typename Vertex>
  std::vector<Vertex> Tessellate(Vertex make_vertex(const Point& point)) const {
    std::vector<Vertex> vertices;
    vertices.reserve(14);
    vertices.push_back(make_vertex(a_[0]));
    vertices.push_back(make_vertex(a_[1]));
    vertices.push_back(make_vertex(a_[3]));
    vertices.push_back(make_vertex(a_[2]));
    vertices.push_back(make_vertex(b_[2]));
    vertices.push_back(make_vertex(a_[1]));
    vertices.push_back(make_vertex(b_[1]));
    vertices.push_back(make_vertex(a_[0]));
    vertices.push_back(make_vertex(b_[0]));
    vertices.push_back(make_vertex(a_[3]));
    vertices.push_back(make_vertex(b_[3]));
    vertices.push_back(make_vertex(b_[2]));
    vertices.push_back(make_vertex(b_[0]));
    vertices.push_back(make_vertex(b_[1]));
    return vertices;
  }

 private:
  Quad a_;
  Quad b_;
};

}  // namespace vfx

#endif  // VFX_GEOMETRY_CUBOID_H_
