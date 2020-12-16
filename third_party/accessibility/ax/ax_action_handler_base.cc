// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_action_handler_base.h"

#include "flutter/fml/logging.h"

#include "ax_tree_id_registry.h"

namespace ax {

bool AXActionHandlerBase::RequiresPerformActionPointInPixels() const {
  return false;
}

AXActionHandlerBase::AXActionHandlerBase()
    : AXActionHandlerBase(ax::AXTreeIDUnknown()) {}

AXActionHandlerBase::AXActionHandlerBase(const AXTreeID& ax_tree_id)
    : tree_id_(ax_tree_id) {}

AXActionHandlerBase::~AXActionHandlerBase() {
  AXTreeIDRegistry::GetInstance().RemoveAXTreeID(tree_id_);
}

void AXActionHandlerBase::SetAXTreeID(AXTreeID new_ax_tree_id) {
  FML_DCHECK(new_ax_tree_id != ax::AXTreeIDUnknown());
  AXTreeIDRegistry::GetInstance().RemoveAXTreeID(tree_id_);
  tree_id_ = new_ax_tree_id;
  AXTreeIDRegistry::GetInstance().SetAXTreeID(tree_id_, this);
}

}  // namespace ax
