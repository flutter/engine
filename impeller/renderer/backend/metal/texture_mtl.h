// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <Metal/Metal.h>
#include <QuartzCore/QuartzCore.h>

#include "flutter/fml/macros.h"
#include "fml/synchronization/count_down_latch.h"
#include "impeller/base/backend_cast.h"
#include "impeller/core/texture.h"

namespace impeller {

class TextureMTL : public Texture, public BackendCast<TextureMTL, Texture> {
 public:
  TextureMTL(TextureDescriptor desc,
             id<MTLTexture> texture,
             bool wrapped = false,
             CAMetalLayer* layer = nil);

  static std::shared_ptr<TextureMTL> Wrapper(
      TextureDescriptor desc,
      CAMetalLayer* layer,
      std::function<void()> deletion_proc = nullptr);

  // |Texture|
  ~TextureMTL() override;

  id<MTLTexture> GetMTLTexture() const;

  bool IsWrapped() const;

  bool GenerateMipmap(id<MTLBlitCommandEncoder> encoder);

  void WaitForNextDrawable() {
    if (!is_wrapped_) {
      return;
    }
    latch_->Wait();
  }

  id<CAMetalDrawable> GetDrawable() const { return drawable_; }

 private:
  mutable id<MTLTexture> texture_ = nullptr;
  CAMetalLayer* layer_ = nil;
  mutable id<CAMetalDrawable> drawable_ = nil;
  mutable std::shared_ptr<fml::CountDownLatch> latch_;

  bool is_valid_ = false;
  bool is_wrapped_ = false;

  // |Texture|
  void SetLabel(std::string_view label) override;

  // |Texture|
  bool OnSetContents(const uint8_t* contents,
                     size_t length,
                     size_t slice) override;

  // |Texture|
  bool OnSetContents(std::shared_ptr<const fml::Mapping> mapping,
                     size_t slice) override;

  // |Texture|
  bool IsValid() const override;

  // |Texture|
  ISize GetSize() const override;

  TextureMTL(const TextureMTL&) = delete;

  TextureMTL& operator=(const TextureMTL&) = delete;
};

}  // namespace impeller
