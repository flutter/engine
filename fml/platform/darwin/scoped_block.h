// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FML_PLATFORM_DARWIN_SCOPED_BLOCK_H_
#define FLUTTER_FML_PLATFORM_DARWIN_SCOPED_BLOCK_H_

#include <Block.h>

#include "flutter/fml/compiler_specific.h"

namespace fml {

static_assert(
    __has_feature(objc_arc),
    "Only ARC enabled translation units may use/include these utilities.");

template <typename B>
class ScopedBlock {
 public:
  explicit ScopedBlock(B block = nullptr) : block_(block) {}

  ScopedBlock(const ScopedBlock<B>& that) : block_(that.block_) {}

  ~ScopedBlock() = default;

  ScopedBlock& operator=(const ScopedBlock<B>& that) {
    reset(that.get());
    return *this;
  }

  void reset(B block = nullptr) { block_ = block; }

  bool operator==(B that) const { return block_ == that; }

  bool operator!=(B that) const { return block_ != that; }

  operator B() const { return block_; }

  B get() const { return block_; }

  void swap(ScopedBlock& that) {
    B temp = that.block_;
    that.block_ = block_;
    block_ = temp;
  }

  B release() FML_WARN_UNUSED_RESULT {
    B temp = block_;
    block_ = nullptr;
    return temp;
  }

 private:
  B block_;
};

}  // namespace fml

#endif  // FLUTTER_FML_PLATFORM_DARWIN_SCOPED_BLOCK_H_
