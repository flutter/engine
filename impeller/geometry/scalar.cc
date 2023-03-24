// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "scalar.h"

namespace impeller {

uint16_t ScalarToHalf(Scalar f) {
  uint32_t x = *reinterpret_cast<const uint32_t*>(&f);
  uint32_t sign = (unsigned short)(x >> 31);
  uint32_t mantissa;
  uint32_t exp;
  uint16_t hf;

  mantissa = x & ((1 << 23) - 1);
  exp = x & (0xFF << 23);
  if (exp >= 0x47800000) {
    // check if the original number is a NaN
    if (mantissa && (exp == (0xFF << 23))) {
      // single precision NaN
      mantissa = (1 << 23) - 1;
    } else {
      // half-float will be Inf
      mantissa = 0;
    }
    hf = (((uint16_t)sign) << 15) | (uint16_t)((0x1F << 10)) |
         (uint16_t)(mantissa >> 13);
  }
  // check if exponent is <= -15
  else if (exp <= 0x38000000) {
    hf = 0;  // too small to be represented
  } else {
    hf = (((uint16_t)sign) << 15) | (uint16_t)((exp - 0x38000000) >> 13) |
         (uint16_t)(mantissa >> 13);
  }
  return hf;
}

}  // namespace impeller
