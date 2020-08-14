// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterWindowController.h"

@implementation FlutterWindowController

- (id)initWithView:(nonnull NSView*)view
    reshapeListener:(nonnull id<FlutterViewReshapeListener>)reshapeListener {
  _view = view;
  [view.window setDelegate:self];
  _reshapeListener = reshapeListener;
  return self;
}

#pragma mark - NSWindowDelegate overrides.

- (NSSize)windowWillResize:(NSWindow*)sender toSize:(NSSize)frameSize {
  [_reshapeListener viewDidReshape:_view];
  return frameSize;
}

- (void)windowDidResize:(NSNotification*)notification {
  // Calls to windowWillResize:toSize: are not matched evenly with calls to windowDidResize: because
  // the former is NOT called in the event that you are setting the window's size with
  // setContentSize: or setFrame:, but windowDidResize: is. Hence we need to update the  viewport
  // metrics on windowDidResize as well.
  [_reshapeListener viewDidReshape:_view];
}

@end
