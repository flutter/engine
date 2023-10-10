// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

#include "flutter/shell/platform/embedder/embedder.h"

#include <vector>

@class FlutterView;
@class FlutterMutatorView;

/// FlutterCursorCoordinator is responsible for coordinating cursor changes between
/// platform views and overlays of single FlutterView.
@interface FlutterCursorCoordinator : NSObject

- (nonnull FlutterCursorCoordinator*)initWithFlutterView:(nonnull FlutterView*)flutterView;

@end

/// Exposed methods for testing.
@interface FlutterCursorCoordinator (Private)

@property(readonly, nonatomic) BOOL cleanupScheduled;

- (void)processMouseMoveEvent:(nonnull NSEvent*)event
               forMutatorView:(nonnull FlutterMutatorView*)view
                overlayRegion:(const std::vector<CGRect>&)region;
@end

/// FlutterMutatorView contains platform view and is responsible for applying
/// FlutterLayer mutations to it.
@interface FlutterMutatorView : NSView

/// Designated initializer.
- (nonnull instancetype)initWithPlatformView:(nonnull NSView*)platformView
                            cursorCoordiator:(nullable FlutterCursorCoordinator*)coordinator;

- (nonnull instancetype)initWithPlatformView:(nonnull NSView*)platformView;

/// Returns wrapped platform view.
@property(readonly, nonnull) NSView* platformView;

/// Applies mutations from FlutterLayer to the platform view. This may involve
/// creating or removing intermediate subviews depending on current state and
/// requested mutations.
- (void)applyFlutterLayer:(nonnull const FlutterLayer*)layer;

/// Resets hit hit testing region for this mutator view.
- (void)resetHitTestRegion;

/// Adds rectangle (in local vie coordinates) to hit test ignore region
/// (part of view obscured by Flutter contents).
- (void)addHitTestIgnoreRegion:(CGRect)region;

@end
