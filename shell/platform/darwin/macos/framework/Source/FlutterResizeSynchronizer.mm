#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterResizeSynchronizer.h"

#import <mutex>

@interface FlutterResizeSynchronizer () {
  uint32_t cookie;  // counter to detect stale callbacks
  std::mutex mutex;
  std::condition_variable condRun;
  std::condition_variable condBlock;
  bool accepting;
  bool waiting;
  bool pendingCommit;
  CGSize newSize;
  __weak id<FlutterResizeSynchronizerDelegate> delegate;
}
@end

@implementation FlutterResizeSynchronizer

- (instancetype)initWithDelegate:(id<FlutterResizeSynchronizerDelegate>)delegate_ {
  if (self = [super init]) {
    accepting = true;
    delegate = delegate_;
  }
  return self;
}

- (void)beginResize:(CGSize)size notify:(dispatch_block_t)notify {
  std::unique_lock<std::mutex> lock(mutex);
  if (!delegate) {
    return;
  }

  ++cookie;

  // from now on, ignore all incoming commits until the block below gets
  // scheduled on raster thread
  accepting = false;

  // let pending commits finish to unblock the raster thread
  condRun.notify_all();

  // let the engine send resize notification
  notify();

  newSize = size;

  waiting = true;

  condBlock.wait(lock);

  if (pendingCommit) {
    [delegate resizeSynchronizerCommit:self];
    pendingCommit = false;
    condRun.notify_all();
  }

  waiting = false;
}

- (bool)shouldEnsureSurfaceForSize:(CGSize)size {
  std::unique_lock<std::mutex> lock(mutex);
  if (!accepting) {
    if (CGSizeEqualToSize(newSize, size)) {
      accepting = true;
    }
  }
  return accepting;
}

- (void)requestCommit {
  std::unique_lock<std::mutex> lock(mutex);
  if (!accepting) {
    return;
  }

  if (waiting) {  // BeginResize is in progress, interrupt it and schedule commit call
    pendingCommit = true;
    condBlock.notify_all();
    condRun.wait(lock);
  } else {
    // No resize, schedule commit on platform thread and wait until either done
    // or interrupted by incoming BeginResize
    dispatch_async(dispatch_get_main_queue(), [self, cookie_ = cookie] {
      std::unique_lock<std::mutex> lock(mutex);
      if (cookie_ == cookie) {
        if (delegate) {
          [delegate resizeSynchronizerCommit:self];
        }
        condRun.notify_all();
      }
    });
    condRun.wait(lock);
  }
}

@end
