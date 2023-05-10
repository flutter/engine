// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/testing/mock_texture.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/testing/display_list_testing.h"

namespace flutter {
namespace testing {

MockTexture::MockTexture(int64_t textureId) : Texture(textureId) {}

void MockTexture::Paint(PaintContext& context,
                        const SkRect& bounds,
                        bool freeze,
                        DlImageSampling sampling) {
  DlPaint paint;
  if (context.paint) {
    paint = *context.paint;
  }
  paint.setColor(mockColor(paint.getAlpha(), freeze, sampling));
  context.canvas->DrawRect(bounds, paint);
}

DlColor MockTexture::mockColor(uint8_t alpha,
                               bool freeze,
                               DlImageSampling sampling) const {
  uint8_t red = Id() & 0xff;
  uint8_t green = freeze ? 1 : 0;
  uint8_t blue = static_cast<uint8_t>(sampling);
  return DlColor(alpha << 24 | red << 16 | green << 8 | blue);
}

}  // namespace testing
}  // namespace flutter
