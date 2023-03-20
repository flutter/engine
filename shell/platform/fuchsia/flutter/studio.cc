// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "studio.h"

#include <fcntl.h>
#include <lib/fdio/watcher.h>
#include <lib/zx/time.h>
#include <unistd.h>

#include "flutter/fml/unique_fd.h"

namespace flutter_runner {

Studio::Studio(GrDirectContext* gr_context) : gr_context_(gr_context) {}

Studio::~Studio() = default;

// |flutter::Studio|
bool Studio::IsValid() {
  return true;
}

// |flutter::Studio|
GrDirectContext* Studio::GetContext() {
  return gr_context_;
}

}  // namespace flutter_runner
