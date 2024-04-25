// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_TOOLKIT_WASM_REACTOR_WORKER_H_
#define FLUTTER_IMPELLER_TOOLKIT_WASM_REACTOR_WORKER_H_

#include "impeller/renderer/backend/gles/reactor_gles.h"

namespace impeller::wasm {

class ReactorWorker final : public impeller::ReactorGLES::Worker {
 public:
  ReactorWorker();

  // |ReactorGLES::Worker|
  ~ReactorWorker() override;

  ReactorWorker(const ReactorWorker&) = delete;

  ReactorWorker& operator=(const ReactorWorker&) = delete;

  // |ReactorGLES::Worker|
  bool CanReactorReactOnCurrentThreadNow(
      const ReactorGLES& reactor) const override;
};

}  // namespace impeller::wasm

#endif  // FLUTTER_IMPELLER_TOOLKIT_WASM_REACTOR_WORKER_H_
