// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <vector>

#include "flutter/fml/macros.h"
#include "impeller/base/backend_cast.h"
#include "impeller/renderer/backend/vulkan/vk.h"
#include "impeller/renderer/device_buffer.h"
#include "impeller/renderer/texture.h"
#include "vulkan/vulkan_structs.hpp"

namespace impeller {

class TextureVK final : public Texture, public BackendCast<TextureVK, Texture> {
 public:
  TextureVK(TextureDescriptor desc,
            vk::Image image,
            vk::UniqueImageView image_view,
            vk::Format image_format,
            vk::Extent2D extent);

  // |Texture|
  ~TextureVK() override;

 private:
  vk::Image image_;
  vk::UniqueImageView image_view_;
  vk::Format image_format_;
  vk::Extent2D extent_;

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

  FML_DISALLOW_COPY_AND_ASSIGN(TextureVK);
};

}  // namespace impeller
