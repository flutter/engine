// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/skia_event_tracer_impl.h"

#include "flutter/fml/trace_event.h"
#include "third_party/skia/include/utils/SkEventTracer.h"

namespace skia {

// WIP

}  // namespace skia

void InitSkiaEventTracer() {
#if 0  // WIP
  // Initialize the binding to Skia's tracing events. Skia will
  // take ownership of and clean up the memory allocated here.
  SkEventTracer::SetInstance(new skia::SkChromiumEventTracer());
#endif
}
