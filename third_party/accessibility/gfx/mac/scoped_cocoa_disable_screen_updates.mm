// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/mac/scoped_cocoa_disable_screen_updates.h"

#import <Cocoa/Cocoa.h>

#include "base/mac/mac_util.h"

namespace gfx {

ScopedCocoaDisableScreenUpdates::ScopedCocoaDisableScreenUpdates() {
  if (base::mac::IsAtLeastOS10_11()) {
    // Beginning with OS X 10.11, [NSAnimationContext beginGrouping] is the
    // preferred way of disabling screen updates. Use of
    // NSDisableScreenUpdates() is discouraged.
    [NSAnimationContext beginGrouping];
  } else {
    NSDisableScreenUpdates();
  }
}

ScopedCocoaDisableScreenUpdates::~ScopedCocoaDisableScreenUpdates() {
  if (base::mac::IsAtLeastOS10_11()) {
    [NSAnimationContext endGrouping];
  } else {
    NSEnableScreenUpdates();
  }
}

}  // namespace gfx
