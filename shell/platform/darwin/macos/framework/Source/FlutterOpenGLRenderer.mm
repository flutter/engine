// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterOpenGLRenderer.h"
#import "flutter/shell/platform/darwin/embedder/FlutterEmbedderEngine.h"

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterExternalTextureGL.h"

#pragma mark - Static methods for openGL callbacks that require the engine.

static bool OnMakeCurrent(FlutterEmbedderEngine* engine) {
  FlutterOpenGLRenderer* openGLRenderer = reinterpret_cast<FlutterOpenGLRenderer*>(engine.renderer);
  return [openGLRenderer makeCurrent];
}

static bool OnClearCurrent(FlutterEmbedderEngine* engine) {
  FlutterOpenGLRenderer* openGLRenderer = reinterpret_cast<FlutterOpenGLRenderer*>(engine.renderer);
  return [openGLRenderer clearCurrent];
}

static bool OnPresent(FlutterEmbedderEngine* engine) {
  return [engine.renderer present];
}

static uint32_t OnFBO(FlutterEmbedderEngine* engine, const FlutterFrameInfo* info) {
  FlutterOpenGLRenderer* openGLRenderer = reinterpret_cast<FlutterOpenGLRenderer*>(engine.renderer);
  return [openGLRenderer fboForFrameInfo:info];
}

static bool OnMakeResourceCurrent(FlutterEmbedderEngine* engine) {
  FlutterOpenGLRenderer* openGLRenderer = reinterpret_cast<FlutterOpenGLRenderer*>(engine.renderer);
  return [openGLRenderer makeResourceCurrent];
}

static bool OnAcquireExternalTexture(FlutterEmbedderEngine* engine,
                                     int64_t textureIdentifier,
                                     size_t width,
                                     size_t height,
                                     FlutterOpenGLTexture* openGlTexture) {
  FlutterOpenGLRenderer* openGLRenderer = reinterpret_cast<FlutterOpenGLRenderer*>(engine.renderer);
  return [openGLRenderer populateTextureWithIdentifier:textureIdentifier
                                         openGLTexture:openGlTexture];
}

#pragma mark - FlutterOpenGLRenderer implementation.

@implementation FlutterOpenGLRenderer {
  NSObject<FlutterPresenter>* _flutterView;

  // The context provided to the Flutter engine for rendering to the FlutterView. This is lazily
  // created during initialization of the FlutterView. This is used to render content into the
  // FlutterView.
  NSOpenGLContext* _openGLContext;

  // The context provided to the Flutter engine for resource loading.
  NSOpenGLContext* _resourceContext;
}

- (instancetype)init {
  self = [super initWithDelegate:self];
  return self;
}

- (void)setFlutterView:(NSObject<FlutterPresenter>*)view {
  _flutterView = view;
  if (!view) {
    _resourceContext = nil;
  }
}

- (BOOL)makeCurrent {
  if (!_openGLContext) {
    return false;
  }
  [_openGLContext makeCurrentContext];
  return true;
}

- (BOOL)clearCurrent {
  [NSOpenGLContext clearCurrentContext];
  return true;
}

- (BOOL)present {
  if (!_openGLContext || !_flutterView) {
    return NO;
  }
  [_flutterView present];
  return YES;
}

- (void)presentWithoutContent {
  if (!_flutterView) {
    return;
  }
  [_flutterView presentWithoutContent];
}

- (uint32_t)fboForFrameInfo:(const FlutterFrameInfo*)info {
  CGSize size = CGSizeMake(info->size.width, info->size.height);
  FlutterOpenGLRenderBackingStore* backingStore =
      reinterpret_cast<FlutterOpenGLRenderBackingStore*>([_flutterView backingStoreForSize:size]);
  return backingStore.frameBufferID;
}

- (NSOpenGLContext*)resourceContext {
  if (!_resourceContext) {
    NSOpenGLPixelFormatAttribute attributes[] = {
        NSOpenGLPFAColorSize, 24, NSOpenGLPFAAlphaSize, 8, 0,
    };
    NSOpenGLPixelFormat* pixelFormat = [[NSOpenGLPixelFormat alloc] initWithAttributes:attributes];
    _resourceContext = [[NSOpenGLContext alloc] initWithFormat:pixelFormat shareContext:nil];
  }
  return _resourceContext;
}

- (NSOpenGLContext*)openGLContext {
  if (!_openGLContext) {
    NSOpenGLContext* shareContext = [self resourceContext];
    _openGLContext = [[NSOpenGLContext alloc] initWithFormat:shareContext.pixelFormat
                                                shareContext:shareContext];
  }
  return _openGLContext;
}

- (BOOL)makeResourceCurrent {
  [self.resourceContext makeCurrentContext];
  return YES;
}

#pragma mark - FlutterTextureRegistrar

- (BOOL)populateTextureWithIdentifier:(int64_t)textureID
                        openGLTexture:(FlutterOpenGLTexture*)openGLTexture {
  id<FlutterExternalTexture> texture = [self getTextureWithID:textureID];
  FlutterExternalTextureGL* glTexture = reinterpret_cast<FlutterExternalTextureGL*>(texture);
  return [glTexture populateTexture:openGLTexture];
}

- (id<FlutterExternalTexture>)onRegisterTexture:(id<FlutterTexture>)texture {
  return [[FlutterExternalTextureGL alloc] initWithFlutterTexture:texture];
}

- (FlutterRendererConfig)createRendererConfig {
  const FlutterRendererConfig rendererConfig = {
      .type = kOpenGL,
      .open_gl.struct_size = sizeof(FlutterOpenGLRendererConfig),
      .open_gl.make_current = reinterpret_cast<BoolCallback>(OnMakeCurrent),
      .open_gl.clear_current = reinterpret_cast<BoolCallback>(OnClearCurrent),
      .open_gl.present = reinterpret_cast<BoolCallback>(OnPresent),
      .open_gl.fbo_with_frame_info_callback = reinterpret_cast<UIntFrameInfoCallback>(OnFBO),
      .open_gl.fbo_reset_after_present = true,
      .open_gl.make_resource_current = reinterpret_cast<BoolCallback>(OnMakeResourceCurrent),
      .open_gl.gl_external_texture_frame_callback =
          reinterpret_cast<TextureFrameCallback>(OnAcquireExternalTexture),
  };
  return rendererConfig;
}

@end
