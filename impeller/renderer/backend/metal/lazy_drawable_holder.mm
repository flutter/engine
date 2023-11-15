// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/renderer/backend/metal/lazy_drawable_holder.h"

#include <QuartzCore/CAMetalLayer.h>

#include "flutter/fml/trace_event.h"
#include "impeller/base/validation.h"

namespace impeller {

#pragma GCC diagnostic push
// Disable the diagnostic for iOS Simulators. Metal without emulation isn't
// available prior to iOS 13 and that's what the simulator headers say when
// support for CAMetalLayer begins. CAMetalLayer is available on iOS 8.0 and
// above which is well below Flutters support level.
#pragma GCC diagnostic ignored "-Wunguarded-availability-new"

LazyDrawableHolder::LazyDrawableHolder(CAMetalLayer* layer) : layer_(layer) {}

id<CAMetalDrawable> LazyDrawableHolder::GetDrawable() const {
  if (acquired_) {
    return drawable_;
  }
  drawable_latch_->Wait();
  return drawable_;
}

id<MTLTexture> LazyDrawableHolder::AcquireNextDrawable() {
  if (acquired_) {
    return texture_;
  }
  id<CAMetalDrawable> current_drawable = nil;
  {
    TRACE_EVENT0("impeller", "WaitForNextDrawable");
    current_drawable = [layer_ nextDrawable];
  }
  acquired_ = true;
  drawable_latch_->CountDown();
  if (!current_drawable) {
    VALIDATION_LOG << "Could not acquire current drawable.";
    return nullptr;
  }
  drawable_ = current_drawable;
  texture_ = drawable_.texture;

  return texture_;
}

#pragma GCC diagnostic pop

}  // namespace impeller
