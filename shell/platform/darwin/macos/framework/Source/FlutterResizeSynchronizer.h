#import <Cocoa/Cocoa.h>

@class FlutterResizeSynchronizer;

@protocol FlutterResizeSynchronizerDelegate

// Invoked on raster thread; Delegate should flush the OpenGL context
- (void)resizeSynchronizerFlush:(FlutterResizeSynchronizer*)synchronizer;

// Invoked on platform thread; Delegate should flip the surfaces
- (void)resizeSynchronizerCommit:(FlutterResizeSynchronizer*)synchronizer;

@end

// Encapsulates the logic for blocking platform thread during window resize
@interface FlutterResizeSynchronizer : NSObject

- (instancetype)initWithDelegate:(id<FlutterResizeSynchronizerDelegate>)delegate;

// Blocks the platform thread until
// - shouldEnsureSurfaceForSize is called with proper size and
// - requestCommit is called
// All requestCommit calls before `shouldEnsureSurfaceForSize` is called with
// expected size are ignored;
- (void)beginResize:(CGSize)size notify:(dispatch_block_t)notify;

// Returns whether the view should ensure surfaces with given size;
// This will be false during resizing for any size other than size specified
// during beginResize
- (bool)shouldEnsureSurfaceForSize:(CGSize)size;

// Called from rasterizer thread, will block until delegate resizeSynchronizerCommit:
// method is called (on platform thread)
- (void)requestCommit;

@end
