// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <QuartzCore/CAMetalLayer.h>
#include <memory>
#include <optional>

#include "flutter/fml/macros.h"
#include "impeller/geometry/rect.h"
#include "impeller/renderer/backend/metal/lazy_drawable_holder.h"
#include "impeller/renderer/context.h"
#include "impeller/renderer/surface.h"

namespace impeller {

class SurfaceMTL final : public Surface {
 public:
#pragma GCC diagnostic push
  // Disable the diagnostic for iOS Simulators. Metal without emulation isn't
  // available prior to iOS 13 and that's what the simulator headers say when
  // support for CAMetalLayer begins. CAMetalLayer is available on iOS 8.0 and
  // above which is well below Flutters support level.
#pragma GCC diagnostic ignored "-Wunguarded-availability-new"
  //----------------------------------------------------------------------------
  /// @brief      Wraps the given Metal layer to create a surface Impeller can
  ///             render to.
  ///
  /// @param[in]  context  The context
  /// @param[in]    layer  The layer whose current drawable to wrap to create a
  ///                      surface.
  ///
  /// @return     A pointer to the wrapped surface or null.
  static std::unique_ptr<SurfaceMTL> MakeFromMetalLayer(
      const std::shared_ptr<Context>& context,
      CAMetalLayer* layer,
      std::optional<IRect> clip_rect = std::nullopt);

  static std::unique_ptr<SurfaceMTL> MakeFromTexture(
      const std::shared_ptr<Context>& context,
      id<MTLTexture> texture,
      std::optional<IRect> clip_rect = std::nullopt);
#pragma GCC diagnostic pop

  // |Surface|
  ~SurfaceMTL() override;

  // Returns a Rect defining the area of the surface in device pixels
  IRect coverage() const;

  // |Surface|
  bool Present() const override;

 private:
  std::weak_ptr<Context> context_;
  std::shared_ptr<Texture> resolve_texture_;
  std::optional<DeferredDrawable> deferred_drawable_ = std::nullopt;
  std::shared_ptr<Texture> source_texture_;
  std::shared_ptr<Texture> destination_texture_;
  bool requires_blit_ = false;
  std::optional<IRect> clip_rect_;

  static bool ShouldPerformPartialRepaint(std::optional<IRect> damage_rect);

  SurfaceMTL(const std::weak_ptr<Context>& context,
             const RenderTarget& target,
             std::shared_ptr<Texture> resolve_texture,
             const std::optional<DeferredDrawable>& deferred_drawable,
             std::shared_ptr<Texture> source_texture,
             std::shared_ptr<Texture> destination_texture,
             bool requires_blit,
             std::optional<IRect> clip_rect);

  SurfaceMTL(const SurfaceMTL&) = delete;

  SurfaceMTL& operator=(const SurfaceMTL&) = delete;
};

}  // namespace impeller
