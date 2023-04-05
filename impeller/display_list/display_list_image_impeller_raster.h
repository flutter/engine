// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/display_list/image/dl_image.h"
#include "flutter/fml/macros.h"
#include "impeller/core/texture.h"
#include "impeller/renderer/context.h"
#include "third_party/skia/include/core/SkBitmap.h"

namespace impeller {

class AiksContext;

class DlImageImpellerRaster final : public flutter::DlImage {
 public:
  static sk_sp<DlImageImpellerRaster> Make(
      std::weak_ptr<Context> context,
      std::shared_ptr<SkBitmap> bitmap,
      OwningContext owning_context = OwningContext::kIO);

  static std::shared_ptr<Texture> UploadTextureToShared(
      std::shared_ptr<Context> context,
      std::shared_ptr<SkBitmap> bitmap,
      bool create_mips);

  // |DlImage|
  ~DlImageImpellerRaster() override;

  // |DlImage|
  sk_sp<SkImage> skia_image() const override;

  // |DlImage|
  std::shared_ptr<impeller::Texture> impeller_texture() const override;

  // |DlImage|
  bool isOpaque() const override;

  // |DlImage|
  bool isTextureBacked() const override;

  // |DlImage|
  SkISize dimensions() const override;

  // |DlImage|
  size_t GetApproximateByteSize() const override;

  // |DlImage|
  OwningContext owning_context() const override { return owning_context_; }

 private:
  std::weak_ptr<Context> context_;
  std::shared_ptr<SkBitmap> bitmap_;
  mutable std::shared_ptr<Texture> texture_;
  OwningContext owning_context_;

  explicit DlImageImpellerRaster(
      std::weak_ptr<Context> context,
      std::shared_ptr<SkBitmap> bitmap,
      OwningContext owning_context = OwningContext::kIO);

  FML_DISALLOW_COPY_AND_ASSIGN(DlImageImpellerRaster);
};

}  // namespace impeller
