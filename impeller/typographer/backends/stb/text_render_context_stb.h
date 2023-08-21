// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include "impeller/typographer/text_render_context.h"

#include "flutter/fml/macros.h"

namespace impeller {

class TextRenderContextSTB : public TextRenderContext {
 public:
  TextRenderContextSTB(std::shared_ptr<Context> context);

  ~TextRenderContextSTB() override;

  // |TextRenderContext|
  std::shared_ptr<GlyphAtlas> CreateGlyphAtlas(
      GlyphAtlas::Type type,
      std::shared_ptr<GlyphAtlasContext> atlas_context,
      FrameIterator iterator) const override;

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(TextRenderContextSTB);
};

}  // namespace impeller
