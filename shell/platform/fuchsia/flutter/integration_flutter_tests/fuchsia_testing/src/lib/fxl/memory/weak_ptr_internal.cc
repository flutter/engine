// Copyright 2016 The Fuchsia Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "src/lib/fxl/memory/weak_ptr_internal.h"

#include <lib/syslog/cpp/macros.h>

namespace fxl {
namespace internal {

WeakPtrFlag::WeakPtrFlag() : is_valid_(true) {}

WeakPtrFlag::~WeakPtrFlag() {
  // Should be invalidated before destruction.
  FX_DCHECK(!is_valid_);
}

void WeakPtrFlag::Invalidate() {
  // Invalidation should happen exactly once.
  FX_DCHECK(is_valid_);
  is_valid_ = false;
}

}  // namespace internal
}  // namespace fxl
