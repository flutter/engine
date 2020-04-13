// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/common/layer_tree_holder.h"

namespace flutter {

std::unique_ptr<LayerTree> LayerTreeHolder::Get() {
  std::scoped_lock lock(layer_tree_mutex);
  return std::move(layer_tree_);
}

void LayerTreeHolder::ReplaceIfNewer(std::unique_ptr<LayerTree> layer_tree) {
  std::scoped_lock lock(layer_tree_mutex);
  if (IsEmpty()) {
    layer_tree_ = std::move(layer_tree);
  } else if (layer_tree_->target_time() < layer_tree->target_time()) {
    layer_tree_ = std::move(layer_tree);
  }
}

bool LayerTreeHolder::IsEmpty() const {
  std::scoped_lock lock(layer_tree_mutex);
  return !layer_tree_;
}

};  // namespace flutter
