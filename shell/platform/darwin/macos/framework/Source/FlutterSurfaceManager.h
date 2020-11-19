#import <Cocoa/Cocoa.h>

// Manages the IOSurfaces for FlutterView
@interface FlutterSurfaceManager : NSObject

- (instancetype)initWithLayer:(CALayer*)containingLayer
                openGLContext:(NSOpenGLContext*)openGLContext
              numFramebuffers:(int)numFramebuffers;

- (void)ensureSurfaceSize:(CGSize)size;
- (void)swapBuffers;

/**
 * Sets the CALayer content to the content of _ioSurface[kBack].
 */
- (void)setLayerContent;

/**
 * Sets the CALayer content to the content of _ioSurface[index].
 */
- (void)setLayerContentWithIOSurface:(int)index;

/**
 * Recreates the IOSurface _ioSurface[index] with specified size.
 */
- (void)recreateIOSurface:(int)index size:(CGSize)size;

/**
 * Binds the IOSurface at _ioSurface[index] to the texture id at
 * _backingTexture[index] and fbo at _frameBufferId[index].
 */
- (void)backTextureWithIOSurface:(int)index size:(CGSize)size;

/**
 * Returns the kBack framebuffer.
 */
- (uint32_t)glFrameBufferBackId;

/**
 * Returns the kFront framebuffer.
 * The framebuffer is used by FlutterMacOSCompositor to create a backing store.
 * The framebuffer is collected when the backing store that uses the
 * framebuffer is collected.
 */
- (uint32_t)glFrameBufferFrontId;

@end
