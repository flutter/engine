// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <Metal/Metal.h>

#include <future>
#include "impeller/core/texture_descriptor.h"
#include "impeller/renderer/backend/metal/texture_mtl.h"

@protocol CAMetalDrawable;
@class CAMetalLayer;

namespace impeller {

using DeferredDrawable = std::shared_future<id<CAMetalDrawable>>;

#pragma GCC diagnostic push
// Disable the diagnostic for iOS Simulators. Metal without emulation isn't
// available prior to iOS 13 and that's what the simulator headers say when
// support for CAMetalLayer begins. CAMetalLayer is available on iOS 8.0 and
// above which is well below Flutters support level.
#pragma GCC diagnostic ignored "-Wunguarded-availability-new"

/// @brief Create a deferred drawable from a CAMetalLayer.
DeferredDrawable GetDrawableDeferred(CAMetalLayer* layer);

/// @brief Create a TextureMTL from a deferred drawable.
///
///        This function is safe to call multiple times and will only call
///        nextDrawable once.
std::shared_ptr<Texture> CreateTextureFromDrawableFuture(
    TextureDescriptor desc,
    const DeferredDrawable& drawble_future);

#pragma GCC diagnostic pop

}  // namespace impeller
