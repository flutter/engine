// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_PAINT_NODE_H_
#define FLUTTER_FLOW_LAYERS_PAINT_NODE_H_

#include <iterator>
#include <vector>
#include "flutter/display_list/display_list.h"
#include "flutter/display_list/display_list_blend_mode.h"
#include "flutter/display_list/display_list_builder.h"
#include "flutter/display_list/display_list_color_filter.h"
#include "flutter/display_list/display_list_paint.h"

namespace flutter {

class DlPaintNode {
 public:
  explicit DlPaintNode() {}
  virtual ~DlPaintNode() = default;

  virtual void SetSaveLayerAttribute(DlPaint*){};

  DlPaintNode* SetAlpha(uint8_t alpha);

  DlPaintNode* SetColorFilter(const DlColorFilter* color_filter);

  DlPaintNode* SetBackdropFilter(DlBlendMode blend_mode,
                                 const DlImageFilter* filter);

  const DlPaintNode* parent() const { return parent_; }

  bool hasNext() const { return current_index_ >= children_.size(); }

  DlPaintNode* next() {
    if (current_index_ < children_.size()) {
      return children_[current_index_++].get();
    }
    return nullptr;
  }

  std::vector<DlPaintNode*> SaveLayerAttributePaintNodes() {
    DlPaintNode* nodes[depth_];
    auto* current = this;
    auto index = depth_ - 1;
    while (current) {
      nodes[index--] = current;
      current = current->parent_;
    }
    std::vector<DlPaintNode*> a(nodes, nodes + depth_ - 1);
    return a;
  }

 protected:
  std::vector<std::unique_ptr<DlPaintNode>> children_;
  DlPaintNode* parent_;

  unsigned current_index_ = 0;
  unsigned depth_ = 1;
};

class OpacityPaintNode : public DlPaintNode {
 public:
  explicit OpacityPaintNode(uint8_t alpha) : alpha_(alpha) {}

  void SetSaveLayerAttribute(DlPaint*) override;

 private:
  uint8_t alpha_;
};

class BackdropFilterPaintNode : public DlPaintNode {
 public:
  explicit BackdropFilterPaintNode(DlBlendMode blend_mode,
                                   const DlImageFilter* filter)
      : blend_mode_(blend_mode), filter_(filter) {}

  void SetSaveLayerAttribute(DlPaint*) override;

 private:
  DlBlendMode blend_mode_;
  const DlImageFilter* filter_;
};

class ColorFilterPaintNode : public DlPaintNode {
 public:
  explicit ColorFilterPaintNode(const DlColorFilter* color_filter)
      : color_filter_(color_filter) {}

  void SetSaveLayerAttribute(DlPaint*) override;

 private:
  const DlColorFilter* color_filter_;
};

}  // namespace flutter

#endif /* FLUTTER_FLOW_LAYERS_PAINT_NODE_H_ */
