// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "vector.h"

namespace impeller {

template <>
Half Cast<Half, Scalar>(const Scalar& s) {
  return ScalarToHalf(s);
}

template <>
Half Cast<Half, Half>(const Half& s) {
  return s;
}

template <>
Scalar Cast<Scalar, Scalar>(const Scalar& s) {
  return s;
}

}  // namespace impeller
