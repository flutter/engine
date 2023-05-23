// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import <Cocoa/Cocoa.h>

/**
 * Takes care of synchronization between raster and platform thread.
 */
@interface FlutterThreadSynchronizer : NSObject

- (nullable instancetype)init;

/**
 * Called from platform thread. Blocks until a commit with given size (or empty)
 * is requested.
 */
- (void)beginResizeForView:(int64_t)viewId
                      size:(CGSize)size
                    notify:(nonnull dispatch_block_t)notify;

/**
 * Called from raster thread. Schedules the given block on platform thread
 * and blocks until it is performed.
 *
 * If platform thread is blocked in `beginResize:` for given size (or size is empty),
 * unblocks platform thread.
 *
 * The notify block is guaranteed to be called within a core animation transaction.
 */
- (void)performCommitForView:(int64_t)viewId
                        size:(CGSize)size
                      notify:(nonnull dispatch_block_t)notify;

- (void)registerView:(int64_t)viewId;

- (void)deregisterView:(int64_t)viewId;

/**
 * Called from platform thread when shutting down.
 *
 * Prevents any further synchronization and no longer blocks any threads.
 */
- (void)shutdown;

@end

@interface FlutterThreadSynchronizer (Test)

- (nullable instancetype)initWithMainQueue:(nonnull dispatch_queue_t)queue;

/**
 * Blocks current thread until the mutex is available, then return whether the
 * synchronizer is waiting for a correct commit during resizing.
 */
- (BOOL)isWaitingWhenMutexIsAvailable;

/**
 * Blocks current thread until there is frame available.
 * Used in FlutterEngineTest.
 */
- (void)blockUntilFrameAvailable;

@end
