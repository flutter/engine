// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_tree_manager_map.h"

#include "flutter/fml/logging.h"

// #include "base/stl_util.h"
#include "ax_enums.h"

namespace ax {

AXTreeManagerMap::AXTreeManagerMap() {}

AXTreeManagerMap::~AXTreeManagerMap() {}

AXTreeManagerMap& AXTreeManagerMap::GetInstance() {
  static base::NoDestructor<AXTreeManagerMap> instance;
  return *instance;
}

void AXTreeManagerMap::AddTreeManager(AXTreeID tree_id,
                                      AXTreeManager* manager) {
  if (tree_id != AXTreeIDUnknown())
    map_[tree_id] = manager;
}

void AXTreeManagerMap::RemoveTreeManager(AXTreeID tree_id) {
  if (tree_id != AXTreeIDUnknown())
    map_.erase(tree_id);
}

AXTreeManager* AXTreeManagerMap::GetManager(AXTreeID tree_id) {
  if (tree_id == AXTreeIDUnknown() || map_.find(tree_id) != map_.end())
    return nullptr;

  return map_.at(tree_id);
}

AXTreeManager* AXTreeManagerMap::GetManagerForChildTree(
    const AXNode& parent_node) {
  if (!parent_node.data().HasStringAttribute(
          ax::StringAttribute::kChildTreeId)) {
    return nullptr;
  }

  AXTreeID child_tree_id =
      AXTreeID::FromString(parent_node.data().GetStringAttribute(
          ax::StringAttribute::kChildTreeId));
  AXTreeManager* child_tree_manager =
      AXTreeManagerMap::GetInstance().GetManager(child_tree_id);

  // Some platforms do not use AXTreeManagers, so child trees don't exist in
  // the browser process.
  if (!child_tree_manager)
    return nullptr;

  FML_DCHECK(child_tree_manager->GetParentNodeFromParentTreeAsAXNode()->id() ==
         parent_node.id());

  return child_tree_manager;
}

}  // namespace ax
