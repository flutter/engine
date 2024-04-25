// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/toolkit/wasm/reactor_worker.h"

namespace impeller::wasm {

ReactorWorker::ReactorWorker() = default;

ReactorWorker::~ReactorWorker() = default;

// |ReactorGLES::Worker|
bool ReactorWorker::CanReactorReactOnCurrentThreadNow(
    const ReactorGLES& reactor) const {
  return true;
}

}  // namespace impeller::wasm
