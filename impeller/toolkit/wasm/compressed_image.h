// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "flutter/fml/mapping.h"
#include "impeller/core/texture.h"
#include "impeller/renderer/context.h"

namespace impeller::wasm {

std::shared_ptr<Texture> CreateTextureFromCompressedImageData(
    const Context& context,
    const fml::Mapping& mapping);

struct DecompressedImage {
  std::shared_ptr<fml::Mapping> mapping;
  ISize size;
};

DecompressedImage CreateDecompressedTextureMapping(const fml::Mapping& mapping);

}  // namespace impeller::wasm
