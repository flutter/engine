// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_DISPLAY_LIST_LAYER_H_
#define FLUTTER_FLOW_LAYERS_DISPLAY_LIST_LAYER_H_

#include "flutter/flow/display_list.h"
#include "flutter/flow/display_list_utils.h"
#include "flutter/flow/layers/layer.h"
#include "flutter/flow/skia_gpu_object.h"

namespace flutter {

class DisplayListCacheDispatcher : public virtual Dispatcher,
                                   public SkMatrixDispatchHelper,
                                   public IgnoreAttributeDispatchHelper,
                                   public IgnoreClipDispatchHelper,
                                   public IgnoreRenderingDispatchHelper {
 public:
  DisplayListCacheDispatcher(PrerollContext* context, RasterCache* cache)
      : context_(context), cache_(cache) {}

  void save() override { SkMatrixDispatchHelper::save(); }
  void saveLayer(const SkRect* bounds, bool restore_with_paint) override {
    SkMatrixDispatchHelper::save();
  }
  void restore() override { SkMatrixDispatchHelper::restore(); }

  void drawPicture(const sk_sp<SkPicture> picture,
                   const SkMatrix* matrix,
                   bool render_with_attributes) override;
  void drawDisplayList(const sk_sp<DisplayList> display_list) override;

 private:
  PrerollContext* context_;
  RasterCache* cache_;
};

class DisplayListLayer : public Layer {
 public:
  static constexpr size_t kMaxBytesToCompare = 10000;

  DisplayListLayer(const SkPoint& offset,
                   SkiaGPUObject<DisplayList> display_list,
                   bool is_complex,
                   bool will_change);

  DisplayList* display_list() const {
    return display_list_.skia_object().get();
  }

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

  bool IsReplacing(DiffContext* context, const Layer* layer) const override;

  void Diff(DiffContext* context, const Layer* old_layer) override;

  const DisplayListLayer* as_display_list_layer() const override {
    return this;
  }

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

  void Preroll(PrerollContext* frame, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) const override;

 private:
  SkPoint offset_;
  flutter::SkiaGPUObject<DisplayList> display_list_;
  bool is_complex_ = false;
  bool will_change_ = false;

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

  static bool Compare(DiffContext::Statistics& statistics,
                      const DisplayListLayer* l1,
                      const DisplayListLayer* l2);

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

  FML_DISALLOW_COPY_AND_ASSIGN(DisplayListLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_DISPLAY_LIST_LAYER_H_
