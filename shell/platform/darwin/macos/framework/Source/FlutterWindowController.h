// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FLUTTERWINDOWCONTROLLER_H_
#define SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FLUTTERWINDOWCONTROLLER_H_

#import <Cocoa/Cocoa.h>

/**
 * Listener for view resizing.
 */
@protocol FlutterViewReshapeListener <NSObject>
/**
 * Called when the view's backing store changes size or the view bounds change.
 */
- (void)viewDidReshape:(nonnull NSView*)view;
@end

/**
 * Window controller for the main application window.
 */
@interface FlutterWindowController : NSObject <NSWindowDelegate> {
 @private
  __weak NSView* _view;
  __weak id<FlutterViewReshapeListener> _reshapeListener;
}

- (nullable id)initWithView:(nonnull NSView*)view
            reshapeListener:(nonnull id<FlutterViewReshapeListener>)reshapeListener;

@end

#endif  // SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_SOURCE_FLUTTERWINDOWCONTROLLER_H_
