// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/compositor/saturating_counter.h"

namespace sky {

SaturatingCounter::SaturatingCounter() : value_(0) {
}

SaturatingCounter::~SaturatingCounter() {
}

void SaturatingCounter::Increment() {
  if (is_max())
    return;
  ++value_;
}

void SaturatingCounter::Decrement() {
  if (is_min())
    return;
  --value_;
}

}  // namespace sky
