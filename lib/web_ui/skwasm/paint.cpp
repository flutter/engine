// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "export.h"
#include "helpers.h"
#include "render_strategy.h"

using namespace Skwasm;

SKWASM_EXPORT Paint* paint_create() {
  auto paint = new Paint();

  // Antialias defaults to true in flutter.
  paint->setAntiAlias(true);
  return paint;
}

SKWASM_EXPORT void paint_dispose(Paint* paint) {
  delete paint;
}

SKWASM_EXPORT void paint_setBlendMode(Paint* paint, BlendMode mode) {
  paint->setBlendMode(mode);
}

// No getter for blend mode, as it's non trivial. Cache on the dart side.

SKWASM_EXPORT void paint_setStyle(Paint* paint, Paint::Style style) {
  paint->setStyle(style);
}

SKWASM_EXPORT Paint::Style paint_getStyle(Paint* paint) {
  return paint->getStyle();
}

SKWASM_EXPORT void paint_setStrokeWidth(Paint* paint, Scalar width) {
  paint->setStrokeWidth(width);
}

SKWASM_EXPORT Scalar paint_getStrokeWidth(Paint* paint) {
  return paint->getStrokeWidth();
}

SKWASM_EXPORT void paint_setStrokeCap(Paint* paint, Paint::Cap cap) {
  paint->setStrokeCap(cap);
}

SKWASM_EXPORT Paint::Cap paint_getStrokeCap(Paint* paint) {
  return paint->getStrokeCap();
}

SKWASM_EXPORT void paint_setStrokeJoin(Paint* paint, Paint::Join join) {
  paint->setStrokeJoin(join);
}

SKWASM_EXPORT Paint::Join paint_getStrokeJoin(Paint* paint) {
  return paint->getStrokeJoin();
}

SKWASM_EXPORT void paint_setAntiAlias(Paint* paint, bool antiAlias) {
  paint->setAntiAlias(antiAlias);
}

SKWASM_EXPORT bool paint_getAntiAlias(Paint* paint) {
  return paint->isAntiAlias();
}

SKWASM_EXPORT void paint_setColorInt(Paint* paint, Color colorInt) {
  paint->setColor(colorInt);
}

SKWASM_EXPORT Color paint_getColorInt(Paint* paint) {
  return paint->getColor();
}

SKWASM_EXPORT void paint_setMiterLimit(Paint* paint, Scalar miterLimit) {
  paint->setStrokeMiter(miterLimit);
}

SKWASM_EXPORT Scalar paint_getMiterLimit(Paint* paint) {
  return paint->getStrokeMiter();
}

SKWASM_EXPORT void paint_setShader(Paint* paint, SkShader* shader) {
  paint->setShader(sk_ref_sp<SkShader>(shader));
}

SKWASM_EXPORT void paint_setImageFilter(Paint* paint, ImageFilter* filter) {
  //paint->setImageFilter(sk_ref_sp<ImageFilter>(filter));
}

SKWASM_EXPORT void paint_setColorFilter(Paint* paint, ColorFilter* filter) {
  //paint->setColorFilter(sk_ref_sp<ColorFilter>(filter));
}

SKWASM_EXPORT void paint_setMaskFilter(Paint* paint, MaskFilter* filter) {
  //paint->setMaskFilter(sk_ref_sp<MaskFilter>(filter));
}
