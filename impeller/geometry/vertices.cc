// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vertices.h"

namespace impeller {

Vertices::Vertices(std::vector<Point> points,
                   std::vector<uint16_t> indexes,
                   Rect bounds)
    : points_(points), indexes_(indexes), bounds_(bounds){};

Vertices::~Vertices() = default;

std::optional<Rect> Vertices::GetBoundingBox() const {
  return bounds_;
};

}  // namespace impeller
