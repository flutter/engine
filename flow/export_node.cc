// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/export_node.h"

namespace flow {

ExportNode::ExportNode(uint32_t export_token_handle)
    : export_token_(export_token_handle) {}

ExportNode::~ExportNode() {
  // TODO(jeffbrown): Ensure the node is properly unregistered on the rasterizer
  // thread by attaching it to the update context in some way.
}

void ExportNode::Bind(SceneUpdateContext& context,
                      mozart::client::ContainerNode& container,
                      const SkPoint& offset,
                      float scale) {
  ftl::MutexLocker lock(&mutex_);

  if (export_token_) {
    node_.reset(new mozart::client::EntityNode(container.session()));
    node_->Export(std::move(export_token_));
  }

  if (node_) {
    container.AddChild(*node_);
    node_->SetTranslation(offset.x(), offset.y(),
                          10.f);  // FIXME: don't translate in Z
    node_->SetScale(scale, scale, 1.f);
  }
}

}  // namespace flow
