// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_ACCESSIBILITY_PLATFORM_COMPUTE_ATTRIBUTES_H_
#define UI_ACCESSIBILITY_PLATFORM_COMPUTE_ATTRIBUTES_H_

#include <cstddef>
#include <optional>

// #include "base/optional.h"
#include "../ax_enums.h"
#include "../ax_export.h"

namespace ax {

class AXPlatformNodeDelegate;

// Compute the attribute value instead of returning the "raw" attribute value
// for those attributes that have computation methods.
AX_EXPORT std::optional<int32_t> ComputeAttribute(
    const ax::AXPlatformNodeDelegate* delegate,
    ax::IntAttribute attribute);

}  // namespace ax

#endif  // UI_ACCESSIBILITY_PLATFORM_COMPUTE_ATTRIBUTES_H_
