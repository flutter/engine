// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKY_COMPOSITOR_SATURATING_COUNTER_H_
#define SKY_COMPOSITOR_SATURATING_COUNTER_H_

#include "base/macros.h"

namespace sky {

class SaturatingCounter {
 public:
  SaturatingCounter();
  ~SaturatingCounter();

  void Increment();
  void Decrement();

  bool is_min() { return value_ == 0; }
  bool is_max() { return value_ == 3; }

 private:
  int value_ : 2;

  DISALLOW_COPY_AND_ASSIGN(SaturatingCounter);
};

}  // namespace sky

#endif  // SKY_COMPOSITOR_SATURATING_COUNTER_H_
