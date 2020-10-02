// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_GFX_MAC_COORDINATE_CONVERSION_H_
#define UI_GFX_MAC_COORDINATE_CONVERSION_H_

#import <Foundation/Foundation.h>

#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkRect.h"

#include "ax/ax_export.h"

namespace gfx {

// Convert a gfx::Rect specified with the origin at the top left of the primary
// display into AppKit secreen coordinates (origin at the bottom left).
AX_EXPORT NSRect ScreenRectToNSRect(const SkRect& SkRect);

// Convert an AppKit NSRect with origin in the bottom left of the primary
// display into a gfx::Rect with origin at the top left of the primary display.
AX_EXPORT SkRect ScreenRectFromNSRect(const NSRect& point);

// Convert a gfx::Point specified with the origin at the top left of the primary
// display into AppKit screen coordinates (origin at the bottom left).
AX_EXPORT NSPoint ScreenPointToNSPoint(const SkPoint& point);

// Convert an AppKit NSPoint with origin in the bottom left of the primary
// display into a gfx::Point with origin at the top left of the primary display.
AX_EXPORT SkPoint ScreenPointFromNSPoint(const NSPoint& point);

}  // namespace gfx

#endif  // UI_GFX_MAC_COORDINATE_CONVERSION_H_
