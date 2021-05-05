// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Shear.h"
#include <sstream>

namespace rl {
namespace geom {

std::string Shear::toString() const {
  std::stringstream stream;
  stream << "{" << xy << ", " << xz << ", " << yz << "}";
  return stream.str();
}

}  // namespace geom
}  // namespace rl
