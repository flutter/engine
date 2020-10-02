// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "gfx/mac/coordinate_conversion.h"

#import <Cocoa/Cocoa.h>

namespace gfx {

namespace {

// The height of the primary display, which OSX defines as the monitor with the
// menubar. This is always at index 0.
CGFloat PrimaryDisplayHeight() {
  return NSMaxY([[[NSScreen screens] firstObject] frame]);
}

}  // namespace

NSRect ScreenRectToNSRect(const SkRect& rect) {
  return NSMakeRect(rect.x(),
                    PrimaryDisplayHeight() - rect.y() - rect.height(),
                    rect.width(),
                    rect.height());
}

SkRect ScreenRectFromNSRect(const NSRect& rect) {
  return SkRect::MakeXYWH(rect.origin.x,
              PrimaryDisplayHeight() - rect.origin.y - rect.size.height,
              rect.size.width, rect.size.height);
}

NSPoint ScreenPointToNSPoint(const SkPoint& point) {
  return NSMakePoint(point.x(), PrimaryDisplayHeight() - point.y());
}

SkPoint ScreenPointFromNSPoint(const NSPoint& point) {
  return SkPoint::Make(point.x, PrimaryDisplayHeight() - point.y);
}

}  // namespace gfx
