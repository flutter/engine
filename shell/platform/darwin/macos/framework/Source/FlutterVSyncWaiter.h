#import <AppKit/AppKit.h>

@class FlutterDisplayLink;

@interface FlutterVSyncWaiter : NSObject

/// Creates new waiter instance tied to provided NSView.
/// This function must be called on the main thread.
///
/// Provided |block| will be invoked on same thread as -waitForVSync:.
- (instancetype)initWithDisplayLink:(FlutterDisplayLink*)displayLink
                              block:(void (^)(CFTimeInterval timestamp,
                                              CFTimeInterval targetTimestamp,
                                              uintptr_t baton))block;

/// Schedules |baton| to be signaled on next display refresh.
/// The block provided in the initializer will be invoked on same thread
/// as this method (there must be a run loop associated with current thread).
- (void)waitForVSync:(uintptr_t)baton;

@end
