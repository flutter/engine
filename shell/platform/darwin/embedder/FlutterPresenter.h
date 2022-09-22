// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/embedder/FlutterBackingStore.h"

// TODO(cyanglaz embedder api), move this to darwin; support both macos and ios.

@protocol FlutterPresenter

- (void)present;

- (void)presentWithoutContent;

/**
 * Ensures that a backing store with requested size exists and returns the descriptor. Expected to
 * be called on raster thread.
 */
- (nonnull FlutterRenderBackingStore*)backingStoreForSize:(CGSize)size;

@end
