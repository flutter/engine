// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_DISPLAY_LIST_LAYER_H_
#define FLUTTER_FLOW_LAYERS_DISPLAY_LIST_LAYER_H_

#include <memory>

#include "flutter/flow/layers/layer.h"
#include "flutter/flow/raster_cache.h"
#include "flutter/flow/skia_gpu_object.h"
#include "third_party/tonic/typed_data/typed_list.h"
#include "third_party/tonic/typed_data/dart_byte_data.h"

namespace flutter {

class DisplayListLayer : public Layer {
 public:
  DisplayListLayer(const SkPoint& offset,
                   const SkRect& cull_rect,
                   const SkRect& draw_rect,
                   std::shared_ptr<std::vector<uint8_t>> ops,
                   std::shared_ptr<std::vector<float>> data,
                   bool is_complex,
                   bool will_change);

#ifdef FLUTTER_ENABLE_DIFF_CONTEXT

  void Diff(DiffContext* context, const Layer* old_layer) override;

#endif  // FLUTTER_ENABLE_DIFF_CONTEXT

  void Preroll(PrerollContext* frame, const SkMatrix& matrix) override;

  void Paint(PaintContext& context) const override;

 private:
  SkPoint offset_;
  SkRect cull_rect_;
  SkRect draw_rect_;
  std::shared_ptr<std::vector<uint8_t>> ops_vector_;
  std::shared_ptr<std::vector<float>> data_vector_;
  bool is_complex_ = false;
  bool will_change_ = false;

  FML_DISALLOW_COPY_AND_ASSIGN(DisplayListLayer);
};

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_DISPLAY_LIST_LAYER_H_
