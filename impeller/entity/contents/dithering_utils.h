// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_IMPELLER_ENTITY_CONTENTS_DITHERING_UTILS_H_
#define FLUTTER_IMPELLER_ENTITY_CONTENTS_DITHERING_UTILS_H_

#include "impeller/core/allocator.h"
#include "impeller/core/formats.h"
#include "impeller/core/sampler_descriptor.h"
#include "impeller/core/texture.h"
#include "impeller/renderer/blit_pass.h"

namespace impeller {

SamplerDescriptor CreateLUTDescriptor();

/// Generate an 8x8 lookup table for the ordered dither used by gradient
/// shaders.
std::shared_ptr<Texture> GenerateDitherLUT(
    Allocator& allocator,
    PixelFormat pixel_format,
    std::shared_ptr<BlitPass>& blit_pass);

}  // namespace impeller

#endif  // FLUTTER_IMPELLER_ENTITY_CONTENTS_DITHERING_UTILS_H_