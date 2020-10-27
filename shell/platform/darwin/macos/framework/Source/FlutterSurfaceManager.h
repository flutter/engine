#import <Cocoa/Cocoa.h>

// Manages the IOSurfaces for FlutterView
@interface FlutterSurfaceManager : NSObject

- (instancetype)initWithLayer:(CALayer*)layer openGLContext:(NSOpenGLContext*)opengLContext;

- (void)ensureSurfaceSize:(CGSize)size;
- (void)swapBuffers;

- (uint32_t)glFrameBufferId;

/**
 * Sets the CALayer content to the content of _ioSurface[kBack].
 */
- (void)setLayerContent;

/**
 * Sets the CALayer content to the content of the provided ioSurface.
 */
- (void)setLayerContentWithIOSurface:(IOSurfaceRef*)ioSurface;

/**
 * Binds the IOSurface to the provided texture/framebuffer.
 */
- (void)backTextureWithIOSurface:(IOSurfaceRef*)ioSurface
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

/**
 * Returns the kFront IOSurfaceRef.
 * The IOSurface is backed by the FBO provided by getFramebuffer
 * and texture provided by getTexture. The IOSurface is used
 * in FlutterMacOSCompositor's Present call.
 * The IOSurface is collected when the backing store that uses the
 * IOSurface is collected.
 */
- (IOSurfaceRef*)getIOSurface;

@end
