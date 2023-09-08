// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/typographer/text_frame.h"
#include "third_party/skia/include/core/SkTextBlob.h"

namespace impeller {

/// @brief Create an Impeller text frame from a Skia text blob.
///
///  has_color_font controls whether or not the glyphs can be cached in the
///  alpha atlas or the full color atlas.
std::shared_ptr<impeller::TextFrame> MakeTextFrameFromTextBlobSkia(
    const sk_sp<SkTextBlob>& blob,
    bool has_color_font);

/// @brief Testonly version of `MakeTextFrameFromTextBlobSkia` that looks up whether
/// or not the font has bitmap glyphs at runtime.
std::shared_ptr<impeller::TextFrame> MakeTextFrameFromTextBlobSkiaTestOnly(
    const sk_sp<SkTextBlob>& blob);

}  // namespace impeller
