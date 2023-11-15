// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <Metal/Metal.h>

#include <memory>
#include <optional>
#include "fml/synchronization/count_down_latch.h"
#include "impeller/base/thread.h"
#include "impeller/base/thread_safety.h"
#include "impeller/geometry/scalar.h"

@protocol CAMetalDrawable;
@class CAMetalLayer;

namespace impeller {

/// @brief The drawable holder manages requesting and caching the next drawable
///        and its texture from a CAMetalLayer.
class LazyDrawableHolder {
 public:
  explicit LazyDrawableHolder(CAMetalLayer* layer);

  ~LazyDrawableHolder() = default;

  id<MTLTexture> AcquireNextDrawable();

  id<CAMetalDrawable> GetDrawable() const;

 private:
  CAMetalLayer* layer_ = nullptr;
  bool acquired_ = false;
  id<CAMetalDrawable> drawable_ = nullptr;
  id<MTLTexture> texture_ = nullptr;
  std::shared_ptr<fml::CountDownLatch> drawable_latch_ =
      std::make_shared<fml::CountDownLatch>(1u);
};

}  // namespace impeller
