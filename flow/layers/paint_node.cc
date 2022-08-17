// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/paint_node.h"
#include <memory>
#include <utility>

namespace flutter {

DlPaintNode* DlPaintNode::SetAlpha(uint8_t alpha) {
  auto child = std::make_unique<OpacityPaintNode>(alpha);
  child->parent_ = this;
  child->depth_++;
  children_.emplace_back(std::move(child));
  return children_.back().get();
}

DlPaintNode* DlPaintNode::SetColorFilter(const DlColorFilter* color_filter) {
  auto child = std::make_unique<ColorFilterPaintNode>(color_filter);
  child->parent_ = this;
  child->depth_++;
  children_.emplace_back(std::move(child));
  return children_.back().get();
}

DlPaintNode* DlPaintNode::SetBackdropFilter(DlBlendMode blend_mode,
                                            const DlImageFilter* filter) {
  auto child = std::make_unique<BackdropFilterPaintNode>(blend_mode, filter);
  child->parent_ = this;
  child->depth_++;
  children_.emplace_back(std::move(child));
  return children_.back().get();
}

void BackdropFilterPaintNode::SetSaveLayerAttribute(DlPaint* paint) {
  paint->setBlendMode(blend_mode_);
  paint->setImageFilter(filter_);
}

void OpacityPaintNode::SetSaveLayerAttribute(DlPaint* paint) {
  paint->setAlpha(alpha_);
}

void ColorFilterPaintNode::SetSaveLayerAttribute(DlPaint* paint) {
  paint->setColorFilter(color_filter_);
}

}  // namespace flutter
