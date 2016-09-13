// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/glue/allocation_builder.h"

#include <stdlib.h>
#include <string.h>

namespace glue {

AllocationBuilder::AllocationBuilder()
    : buffer_(nullptr), buffer_length_(0), data_length_(0) {}

AllocationBuilder::~AllocationBuilder() {
  free(buffer_);
}

uint32_t AllocationBuilder::Size() const {
  return data_length_;
}

static inline uint32_t RoundPowerOfTwo(uint32_t x) {
  if (x == 0) {
    return 1;
  }

  --x;

  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;

  return x + 1;
}

bool AllocationBuilder::Append(const uint8_t* data, uint32_t length) {
  if (!Reserve(RoundPowerOfTwo(data_length_ + length))) {
    return false;
  }

  memcpy(buffer_ + data_length_, data, length);
  data_length_ += length;

  return true;
}

uint8_t* AllocationBuilder::Take() {
  auto taken = buffer_;
  buffer_ = nullptr;
  buffer_length_ = 0;
  data_length_ = 0;
  return taken;
}

bool AllocationBuilder::Reserve(uint32_t resized_length) {
  if (buffer_length_ == resized_length) {
    return true;
  }

  auto resized_buffer =
      reinterpret_cast<uint8_t*>(realloc(buffer_, resized_length));

  if (resized_buffer == nullptr) {
    // Out of memory.
    return false;
  }

  buffer_ = resized_buffer;
  buffer_length_ = resized_length;

  return true;
}

}  // namespace glue
