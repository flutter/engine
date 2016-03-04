// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef VFX_GEOMETRY_WEDGE_H_
#define VFX_GEOMETRY_WEDGE_H_

#include <vector>

#include "vfx/geometry/plane.h"
#include "vfx/geometry/point.h"
#include "vfx/geometry/triangle.h"

namespace vfx {

// A solid bounded by five planes.
class Wedge {
 public:
  Wedge() { }
  Wedge(const Triangle& a, const Triangle& b) {
    a_ = a;
    b_ = b;
  }

  const Triangle& a() const { return a_; }
  const Triangle& b() const { return b_; }

  void set_a(const Triangle& a) { a_ = a; }
  void set_b(const Triangle& b) { b_ = b; }

  template<typename Vertex>
  std::vector<Vertex> Tessellate(Vertex make_vertex(const Point& point)) const {
    std::vector<Vertex> vertices;
    vertices.reserve(14);
    vertices.push_back(make_vertex(a_[0]));
    vertices.push_back(make_vertex(a_[1]));
    vertices.push_back(make_vertex(a_[2]));
    vertices.push_back(make_vertex(b_[2]));
    vertices.push_back(make_vertex(a_[0]));
    vertices.push_back(make_vertex(b_[0]));
    vertices.push_back(make_vertex(a_[1]));
    vertices.push_back(make_vertex(b_[1]));
    vertices.push_back(make_vertex(b_[2]));
    vertices.push_back(make_vertex(b_[0]));
    return vertices;
  }

  Plane GetPlane(size_t index) const;

 private:
  Triangle a_;
  Triangle b_;
};

}  // namespace vfx

#endif  // VFX_GEOMETRY_WEDGE_H_
