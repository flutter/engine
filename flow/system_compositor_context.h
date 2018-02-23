// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_SYSTEM_COMPOSITOR_CONTEXT_H_
#define FLUTTER_FLOW_SYSTEM_COMPOSITOR_CONTEXT_H_

#include "flutter/flow/compositor_context.h"
#include "flutter/flow/texture.h"
#include "third_party/skia/include/core/SkRect.h"

namespace flow {

class SystemCompositorContext {
 public:
  virtual ~SystemCompositorContext();
  virtual void Reset() = 0;
  virtual void Finish() = 0;
  virtual void PushLayer(SkRect bounds) = 0;
  virtual void PopLayer() = 0;
  virtual SkCanvas* CurrentCanvas() = 0;
  virtual void ClipFrame() = 0;
  virtual void SetColor(SkColor color) = 0;
  virtual void SetClipPath(SkPath path) = 0;
  virtual void AddExternalLayer(Texture* texture, SkRect frame) = 0;
  ;
  virtual void ExecutePaintTasks(CompositorContext::ScopedFrame& frame) = 0;
  virtual void AddPaintedLayer(flow::Layer* layer) = 0;
  virtual void Transform(SkMatrix transform) = 0;
  virtual void PopTransform() = 0;
  TextureRegistry* texture_registry;
};

}  // namespace flow

#endif  // FLUTTER_FLOW_SYSTEM_COMPOSITOR_CONTEXT_H_
