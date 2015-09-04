// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "sky/compositor/picture_state.h"

namespace sky {

PictureState::PictureState() : is_marked_(false) {
}

PictureState::~PictureState() {
}

void PictureState::Mark() {
  is_marked_ = true;
  mark_counter_.Increment();
}

void PictureState::Sweep() {
  if (is_marked_) {
    is_marked_ = false;
    return;
  }

  mark_counter_.Decrement();

  if (mark_counter_.is_min())
    cached_image_ = nullptr;
}

bool PictureState::ShouldRasterize() {
  return mark_counter_.is_max();
}

}  // namespace sky
