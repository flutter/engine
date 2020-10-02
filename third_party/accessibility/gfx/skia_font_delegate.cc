// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/skia_font_delegate.h"

namespace {

gfx::SkiaFontDelegate* g_skia_font_delegate = 0;

}  // namespace

namespace gfx {

void SkiaFontDelegate::SetInstance(SkiaFontDelegate* instance) {
  g_skia_font_delegate = instance;
}

const SkiaFontDelegate* SkiaFontDelegate::instance() {
  return g_skia_font_delegate;
}

}  // namespace gfx
