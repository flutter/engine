// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ACCESSIBILITY_AX_AX_MODE_OBSERVER_H_
#define ACCESSIBILITY_AX_AX_MODE_OBSERVER_H_

#include "ax_export.h"
#include "ax_mode.h"

namespace ax {

class AX_EXPORT AXModeObserver {
 public:
  virtual ~AXModeObserver() {}

  // Notifies when accessibility mode changes.
  virtual void OnAXModeAdded(ax::AXMode mode) = 0;
};

}  // namespace ax

#endif  // ACCESSIBILITY_AX_AX_MODE_OBSERVER_H_
