// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERED_PAINT_CONTEXT_H_
#define FLUTTER_FLOW_LAYERED_PAINT_CONTEXT_H_

#include "third_party/skia/include/core/SkRect.h"
#include "flutter/flow/texture.h"
#include "flutter/flow/compositor_context.h"

namespace flow {

class LayeredPaintContext {
 public:
  virtual ~LayeredPaintContext();
  virtual void Reset() = 0;
  virtual void Finish() = 0;
  virtual void PushLayer(SkRect bounds) = 0;
  virtual void PopLayer() = 0;
  virtual SkCanvas *CurrentCanvas() = 0;
  virtual void ClipRect() = 0;
  virtual void Elevate(float elevation) = 0;
  virtual void SetColor(SkColor color) = 0;
  virtual void SetCornerRadius(float radius) = 0;
  virtual void AddExternalLayer(Texture* texture, SkRect frame) = 0;;
  virtual void ExecutePaintTasks(CompositorContext::ScopedFrame& frame) = 0;
  virtual void AddPaintedLayer(flow::Layer *layer) = 0;
  virtual void Transform(SkMatrix transform) = 0;
  virtual void PopTransform() = 0;
  virtual SkRect SystemCompositedRect() = 0;
  TextureRegistry* texture_registry;
};

} // namespace shell

#endif  // FLUTTER_FLOW_LAYERED_PAINT_CONTEXT_H_
