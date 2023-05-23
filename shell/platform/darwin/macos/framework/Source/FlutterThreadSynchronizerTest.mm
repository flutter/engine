// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterThreadSynchronizer.h"

#import <OCMock/OCMock.h>
#import "flutter/testing/testing.h"
#import "flutter/fml/synchronization/waitable_event.h"

namespace flutter::testing {

namespace {

}  // namespace

}  // namespace flutter::testing

@interface FlutterThreadSynchronizerTestScaffold : NSObject

@property(nonatomic, readonly, nonnull) FlutterThreadSynchronizer* synchronizer;

- (nullable instancetype)init;
- (void)dispatchMainTask:(nonnull void(^)())task;
- (void)dispatchRenderTask:(nonnull void(^)())task;
- (void)joinMain;
- (void)joinRender;
@end

@implementation FlutterThreadSynchronizerTestScaffold {
  dispatch_queue_t _mainQueue;
  std::shared_ptr<fml::AutoResetWaitableEvent> _mainLatch;

  dispatch_queue_t _renderQueue;
  std::shared_ptr<fml::AutoResetWaitableEvent> _renderLatch;

  FlutterThreadSynchronizer* _synchronizer;
}

@synthesize synchronizer = _synchronizer;

- (nullable instancetype)init {
  self = [super init];
  if (self != nil) {
    _mainQueue = dispatch_queue_create("MAIN", DISPATCH_QUEUE_SERIAL);
    _renderQueue = dispatch_queue_create("RENDER", DISPATCH_QUEUE_SERIAL);
    _synchronizer = [[FlutterThreadSynchronizer alloc] initWithMainQueue:_mainQueue];
  }
  return self;
}

- (void)dispatchMainTask:(nonnull void(^)())task {
  dispatch_async(_mainQueue, task);
}

- (void)dispatchRenderTask:(nonnull void(^)())task {
  dispatch_async(_renderQueue, task);
}

- (void)joinMain {
  fml::AutoResetWaitableEvent latch;
  fml::AutoResetWaitableEvent* pLatch = &latch;
  dispatch_async(_mainQueue, ^{ pLatch->Signal(); });
  latch.Wait();
}

- (void)joinRender {
  fml::AutoResetWaitableEvent latch;
  fml::AutoResetWaitableEvent* pLatch = &latch;
  dispatch_async(_renderQueue, ^{ pLatch->Signal(); });
  latch.Wait();
}

@end

TEST(FlutterThreadSynchronizerTest, FinishResizingImmediatelyWhenSizeMatches) {
  FlutterThreadSynchronizerTestScaffold* scaffold = [[FlutterThreadSynchronizerTestScaffold alloc] init];
  FlutterThreadSynchronizer* synchronizer = scaffold.synchronizer;

  [synchronizer registerView:1];

  // Initial resize: does not block until the first frame.
  __block int notifiedResize = 0;
  [scaffold dispatchMainTask:^{
    [synchronizer beginResizeForView:1 size:CGSize{5, 5} notify:^{
      notifiedResize += 1;
    }];
  }];
  EXPECT_FALSE([synchronizer isWaitingWhenMutexIsAvailable]);
  [scaffold joinMain];
  EXPECT_EQ(notifiedResize, 1);

  // Still does not block.
  [scaffold dispatchMainTask:^{
    [synchronizer beginResizeForView:1 size:CGSize{7, 7} notify:^{
      notifiedResize += 1;
    }];
  }];
  EXPECT_FALSE([synchronizer isWaitingWhenMutexIsAvailable]);
  [scaffold joinMain];
  EXPECT_EQ(notifiedResize, 2);

  // First frame
  __block int notifiedCommit = 0;
  [scaffold dispatchRenderTask:^{
    [synchronizer performCommitForView:1 size:CGSize{7, 7} notify:^{
      notifiedCommit += 1;
    }];
  }];
  EXPECT_FALSE([synchronizer isWaitingWhenMutexIsAvailable]);
  [scaffold joinRender];
  EXPECT_EQ(notifiedCommit, 1);
}

TEST(FlutterThreadSynchronizerTest, FinishResizingOnlyWhenCommittingMatchingSize) {
  FlutterThreadSynchronizerTestScaffold* scaffold = [[FlutterThreadSynchronizerTestScaffold alloc] init];
  FlutterThreadSynchronizer* synchronizer = scaffold.synchronizer;
  fml::AutoResetWaitableEvent begunResizingLatch;
  __block fml::AutoResetWaitableEvent* begunResizing = &begunResizingLatch;

  [synchronizer registerView:1];

  // Initial resize: does not block until the first frame.
  [scaffold dispatchMainTask:^{
    [synchronizer beginResizeForView:1 size:CGSize{5, 5} notify:^{}];
  }];
  [scaffold joinMain];
  EXPECT_FALSE([synchronizer isWaitingWhenMutexIsAvailable]);

  // First frame.
  [scaffold dispatchRenderTask:^{
    [synchronizer performCommitForView:1 size:CGSize{5, 5} notify:^{}];
  }];
  [scaffold joinRender];
  EXPECT_FALSE([synchronizer isWaitingWhenMutexIsAvailable]);

  // Resize to (7, 7): blocks since there's a frame until the next frame.
  [scaffold dispatchMainTask:^{
    [synchronizer beginResizeForView:1 size:CGSize{7, 7} notify:^{
      begunResizing->Signal();
    }];
  }];
  begunResizing->Wait();
  EXPECT_TRUE([synchronizer isWaitingWhenMutexIsAvailable]);

  // Render with old size.
  [scaffold dispatchRenderTask:^{
    [synchronizer performCommitForView:1 size:CGSize{5, 5} notify:^{}];
  }];
  [scaffold joinRender];
  EXPECT_TRUE([synchronizer isWaitingWhenMutexIsAvailable]);

  // Render with new size.
  [scaffold dispatchRenderTask:^{
    [synchronizer performCommitForView:1 size:CGSize{7, 7} notify:^{}];
  }];
  [scaffold joinRender];
  EXPECT_FALSE([synchronizer isWaitingWhenMutexIsAvailable]);

  [scaffold joinMain];
}
