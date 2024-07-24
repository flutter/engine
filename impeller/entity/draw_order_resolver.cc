// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/entity/draw_order_resolver.h"

#include "flutter/fml/logging.h"
#include "impeller/base/validation.h"

namespace impeller {

DrawOrderResolver::DrawOrderResolver() : draw_order_layers_({{}}){};

void DrawOrderResolver::AddElement(size_t element_index, bool is_opaque) {
  DrawOrderLayer& layer = draw_order_layers_.back();
  if (is_opaque) {
    layer.opaque_elements.push_back(element_index);
  } else {
    layer.dependent_elements.push_back(element_index);
  }
}
void DrawOrderResolver::PushClip(size_t element_index) {
  draw_order_layers_.back().dependent_elements.push_back(element_index);
  draw_order_layers_.push_back({});
};

void DrawOrderResolver::PopClip() {
  if (draw_order_layers_.size() == 1u) {
    // This is likely recoverable, so don't assert.
    VALIDATION_LOG
        << "Attemped to pop the root draw order layer. This is a bug in "
           "`EntityPass`.";
    return;
  }

  DrawOrderLayer& layer = draw_order_layers_.back();
  DrawOrderLayer& parent_layer =
      draw_order_layers_[draw_order_layers_.size() - 2];

  layer.WriteCombinedDraws(parent_layer.dependent_elements, 0, 0);

  draw_order_layers_.pop_back();
}

DrawOrderResolver::ElementRefs DrawOrderResolver::GetSortedDraws(
    size_t opaque_skip_count,
    size_t translucent_skip_count) const {
  FML_DCHECK(draw_order_layers_.size() == 1u);

  ElementRefs sorted_elements;
  draw_order_layers_.back().WriteCombinedDraws(
      sorted_elements, opaque_skip_count, translucent_skip_count);

  return sorted_elements;
}

void DrawOrderResolver::DrawOrderLayer::WriteCombinedDraws(
    ElementRefs& destination,
    size_t opaque_skip_count,
    size_t translucent_skip_count) const {
  FML_DCHECK(opaque_skip_count <= opaque_elements.size());
  FML_DCHECK(translucent_skip_count <= dependent_elements.size());

  destination.reserve(destination.size() +                          //
                      opaque_elements.size() - opaque_skip_count +  //
                      dependent_elements.size());

  // Draw backdrop-independent elements first.
  destination.insert(destination.end(), opaque_elements.rbegin(),
                     opaque_elements.rend() - opaque_skip_count);
  // Then, draw backdrop-dependent elements in their original order.
  destination.insert(destination.end(),
                     dependent_elements.begin() + translucent_skip_count,
                     dependent_elements.end());
}

}  // namespace impeller
