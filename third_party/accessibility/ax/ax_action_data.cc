// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_action_data.h"

#include "ax_enums.h"

namespace ax {

// Mojo enums are initialized here so the header can include the much smaller
// mojom-forward.h header.
AXActionData::AXActionData()
    : action(ax::Action::kNone),
      hit_test_event_to_fire(ax::Event::kNone),
      horizontal_scroll_alignment(ax::ScrollAlignment::kNone),
      vertical_scroll_alignment(ax::ScrollAlignment::kNone),
      scroll_behavior(ax::ScrollBehavior::kNone) {}

AXActionData::AXActionData(const AXActionData& other) = default;
AXActionData::~AXActionData() = default;

}  // namespace ax
