#import <Cocoa/Cocoa.h>

// Manages the IOSurfaces for FlutterView
@interface FlutterSurfaceManager : NSObject

- (instancetype)initWithLayer:(CALayer*)containingLayer
                openGLContext:(NSOpenGLContext*)openGLContext
              numFramebuffers:(int)numFramebuffers;

- (void)ensureSurfaceSize:(CGSize)size;
- (void)swapBuffers;

- (uint32_t)glFrameBufferId;

/**
 * Sets the CALayer content to the content of _ioSurface[kBack].
 */
- (void)setLayerContent;

/**
 * Sets the CALayer content to the content of ioSurface at ioSurfaceNum.
 */
// TODO(richardjcai): Fix this and remove setLayerContent.
- (void)setLayerContentWithIOSurface:(int)ioSurfaceNum;

/**
 * Binds the IOSurface to the provided texture/framebuffer.
 */
- (void)backTextureWithIOSurface:(int)ioSurfaceNum
                            size:(CGSize)size
                  backingTexture:(GLuint)texture
                             fbo:(GLuint)fbo;

// Methods used by FlutterMacOSCompositor to render Flutter content
// using a single Framebuffer/Texture/IOSurface.

/**
 * Returns the kFront framebuffer.
 * The framebuffer is used by FlutterMacOSCompositor to create a backing store.
 * The framebuffer is collected when the backing store that uses the
 * framebuffer is collected.
 */
- (uint32_t)getFramebuffer;

/**
 * Returns the kFront texture.
 * The texture is used by FlutterMacOSCompositor to create a backing store.
 * The texture is collected when the backing store that uses the
 * texture is collected.
 */
- (uint32_t)getTexture;

@end
