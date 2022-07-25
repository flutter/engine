// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <variant>

#include "impeller/renderer/texture.h"

namespace impeller {

struct BlitCommand {
  struct CopyTextureToTexture {
    std::shared_ptr<Texture> source;
    std::shared_ptr<Texture> destination;
    IRect source_region;
    IPoint destination_origin;
  };

  struct GenerateMipmaps {
    std::shared_ptr<Texture> texture;
  };

  using Variant = std::variant<CopyTextureToTexture, GenerateMipmaps>;

  std::string label;
  Variant data;
};
}  // namespace impeller
