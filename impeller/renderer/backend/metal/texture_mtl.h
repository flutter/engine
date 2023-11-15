// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <Metal/Metal.h>

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/core/texture.h"

@protocol CAMetalDrawable;
@class CAMetalLayer;

namespace impeller {

class TextureMTL : public Texture, public BackendCast<TextureMTL, Texture> {
 public:
  static std::shared_ptr<TextureMTL> Wrapper(
      TextureDescriptor desc,
      id<MTLTexture> texture,
      std::function<void()> deletion_proc = nullptr);

  /// @brief Create a new metal texture.
  static std::shared_ptr<TextureMTL> Create(TextureDescriptor desc,
                                            id<MTLTexture> texture);

#pragma GCC diagnostic push
// Disable the diagnostic for iOS Simulators. Metal without emulation isn't
// available prior to iOS 13 and that's what the simulator headers say when
// support for CAMetalLayer begins. CAMetalLayer is available on iOS 8.0 and
// above which is well below Flutters support level.
#pragma GCC diagnostic ignored "-Wunguarded-availability-new"

  /// @brief Create a new texture backed by a lazily acquired drawable
  static std::shared_ptr<TextureMTL> WrapDrawable(TextureDescriptor desc,
                                                  CAMetalLayer* layer);
#pragma GCC diagnostic pop

  virtual id<MTLTexture> GetMTLTexture() const = 0;

  /// @brief Return the backing Metal drawable (if it has been acquired) or
  ///        otherwise block on drawable aquisition.
  ///
  ///        This is only valud for textures that return true from
  ///        [IsDrawableBacked].
  virtual id<CAMetalDrawable> WaitForNextDrawable() const = 0;

  virtual bool IsWrapped() const = 0;

  /// @brief Whether or not this texture is backed by a CAMetalLayer and needs
  ///        to acquire a drawable before a texture can be returned.
  virtual bool IsDrawableBacked() const = 0;

  virtual bool GenerateMipmap(id<MTLBlitCommandEncoder> encoder) = 0;

 protected:
  explicit TextureMTL(const TextureDescriptor& desc);
};

}  // namespace impeller
