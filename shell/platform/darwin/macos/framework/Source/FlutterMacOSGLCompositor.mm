// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMacOSGLCompositor.h"

#import <OpenGL/gl.h>
#import "flutter/fml/logging.h"
#import "flutter/fml/platform/darwin/cf_utils.h"
#import "flutter/shell/platform/darwin/macos/framework/Source/MacOSSwitchableGLContext.h"
#import "third_party/skia/include/core/SkCanvas.h"
#import "third_party/skia/include/core/SkSurface.h"
#import "third_party/skia/include/gpu/gl/GrGLAssembleInterface.h"
#import "third_party/skia/include/utils/mac/SkCGUtils.h"

#include <unistd.h>

namespace flutter {

FlutterMacOSGLCompositor::FlutterMacOSGLCompositor(FlutterViewController* view_controller)
    : view_controller_(view_controller) {
  auto interface = GrGLMakeNativeInterface();
  GrContextOptions options;
  context_ = GrDirectContext::MakeGL(GrGLMakeNativeInterface(), options);
}

FlutterMacOSGLCompositor::~FlutterMacOSGLCompositor() = default;

bool FlutterMacOSGLCompositor::CreateBackingStore(const FlutterBackingStoreConfig* config,
                                                  FlutterBackingStore* backing_store_out) {
  NSLog(@"FlutterMacOSGLCompositor::CreateBackingStore");
  return CreateFramebuffer(config, backing_store_out);
}

bool FlutterMacOSGLCompositor::CollectBackingStore(const FlutterBackingStore* backing_store) {
  // We have already set the destruction callback for the various backing
  // stores. Our user_data is just the canvas from that backing store and does
  // not need to be explicitly collected. Embedders might have some other state
  // they want to collect though.
  NSLog(@"FlutterMacOSGLCompositor::CollectBackingStore");
  return true;
}

bool FlutterMacOSGLCompositor::Present(const FlutterLayer** layers, size_t layers_count) {
  NSLog(@"layers count: %zu", layers_count);

  for (size_t i = 0; i < layers_count; ++i) {
    const auto* layer = layers[i];
    FlutterBackingStore* backing_store = const_cast<FlutterBackingStore*>(layer->backing_store);
    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore: {
        NSLog(@"kFlutterLayerContentTypeBackingStore, Layer: %zu", i);
        GLuint fbo_id = backing_store->open_gl.framebuffer.name;
        NSLog(@"Presenting FBOID: %u", fbo_id);
        CGSize size = CGSizeMake(layer->size.width, layer->size.height);
        CALayer* content_layer;

        if (backing_store->ca_layer == nullptr) {
          NSLog(@"ca_layer == nullptr");
          content_layer = [[CALayer alloc] init];
          [content_layer
              setFrame:CGRectMake(layer->offset.x, layer->offset.y, size.width, size.height)];
          backing_store->ca_layer = (__bridge void*)content_layer;
          [content_layer setBackgroundColor:[[NSColor yellowColor] CGColor]];
          [view_controller_.flutterView.layer addSublayer:content_layer];
        } else {
          NSLog(@"ca_layer != nullptr");
          content_layer = (__bridge CALayer*)backing_store->ca_layer;
          // [content_layer setFrame:CGRectMake(layer->offset.x, layer->offset.y, size.width,
          // size.height)];
        }

        // valid block 1
        [view_controller_.flutterView getFrameBufferIdForSize:size];
        // valid block end

        IOSurfaceRef ioSurface = reinterpret_cast<IOSurfaceRef>(backing_store->io_surface_ref);
        auto scale = CATransform3DMakeScale(0.5, -0.5, 0.5);
        auto translate = CATransform3DMakeTranslation(size.width * -0.25, size.height * -0.25, 0);
        content_layer.transform = CATransform3DConcat(scale, translate);
        [content_layer setContents:(__bridge id)ioSurface];
        break;
      }
      case kFlutterLayerContentTypePlatformView:
        NSLog(@"kFlutterLayerContentTypePlatformView");
        [view_controller_.view addSubview:view_controller_.view_map.at(1)];
        break;
    };
  }
  // return true;
  return present_callback_();
}

void FlutterMacOSGLCompositor::TextureBackedByIOSurface(const FlutterBackingStoreConfig* config,
                                                        FlutterBackingStore* backing_store_out,
                                                        GLuint texture,
                                                        GLuint fbo) {
  // auto gl_context = view_controller_.flutterView.openGLContext;
  // flutter::GLContextSwitch
  // context_switch(std::make_unique<MacOSSwitchableGLContext>(gl_context));
  IOSurfaceRef ioSurface = reinterpret_cast<IOSurfaceRef>(backing_store_out->io_surface_ref);
  if (ioSurface) {
    CFRelease(ioSurface);
  }
  unsigned pixelFormat = 'BGRA';
  unsigned bytesPerElement = 4;

  auto size = config->size;

  size_t bytesPerRow = IOSurfaceAlignProperty(kIOSurfaceBytesPerRow, size.width * bytesPerElement);
  size_t totalBytes = IOSurfaceAlignProperty(kIOSurfaceAllocSize, size.height * bytesPerRow);
  NSDictionary* options = @{
    (id)kIOSurfaceWidth : @(size.width),
    (id)kIOSurfaceHeight : @(size.height),
    (id)kIOSurfacePixelFormat : @(pixelFormat),
    (id)kIOSurfaceBytesPerElement : @(bytesPerElement),
    (id)kIOSurfaceBytesPerRow : @(bytesPerRow),
    (id)kIOSurfaceAllocSize : @(totalBytes),
  };
  ioSurface = IOSurfaceCreate((CFDictionaryRef)options);
  backing_store_out->io_surface_ref = ioSurface;

  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);

  CGLTexImageIOSurface2D(CGLGetCurrentContext(), GL_TEXTURE_RECTANGLE_ARB, GL_RGBA, int(size.width),
                         int(size.height), GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, ioSurface,
                         0 /* plane */);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_RECTANGLE_ARB, texture,
                         0);

  FML_DCHECK(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
}

bool FlutterMacOSGLCompositor::CreateFramebuffer(const FlutterBackingStoreConfig* config,
                                                 FlutterBackingStore* backing_store_out) {
  NSLog(@"CreateFramebuffer");
  // Logic from FlutterSurfaceManager.
  GrGLFramebufferInfo framebuffer_info = {};
  GLuint texture;

  glGenFramebuffers(1, &framebuffer_info.fFBOID);
  glGenTextures(1, &texture);

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_info.fFBOID);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, texture);
  glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 0);

  // valid block 2 (memory for texture)
  TextureBackedByIOSurface(config, backing_store_out, texture, framebuffer_info.fFBOID);
  // valid block end

  framebuffer_info.fFormat = GL_RGBA8;
  const SkColorType color_type = kN32_SkColorType;

  GrBackendRenderTarget render_target(config->size.width,   // width
                                      config->size.height,  // height
                                      1,                    // sample count
                                      0,                    // stencil bits
                                      framebuffer_info      // framebuffer info
  );

  if (!render_target.isValid()) {
    FML_LOG(ERROR) << "Backend render target was invalid.";
    return false;
  }

  sk_sp<SkColorSpace> colorspace = SkColorSpace::MakeSRGB();
  SkSurfaceProps surface_props(0, kUnknown_SkPixelGeometry);

  auto surface = SkSurface::MakeFromBackendRenderTarget(
      context_.get(),                                // gr context
      render_target,                                 // render target
      GrSurfaceOrigin::kBottomLeft_GrSurfaceOrigin,  // origin
      color_type,                                    // color type
      colorspace,                                    // colorspace
      &surface_props                                 // surface properties
  );

  if (!surface) {
    FML_LOG(ERROR) << "Could not create render target for compositor layer.";
    return false;
  }

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->user_data = surface.get();
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeFramebuffer;
  backing_store_out->open_gl.framebuffer.target = framebuffer_info.fFormat;
  backing_store_out->open_gl.framebuffer.texture = texture;
  backing_store_out->open_gl.framebuffer.name = framebuffer_info.fFBOID;
  // The balancing unref is in the destruction callback.
  surface->ref();
  backing_store_out->open_gl.framebuffer.user_data = surface.get();
  backing_store_out->open_gl.framebuffer.destruction_callback = [](void* user_data) {
    reinterpret_cast<SkSurface*>(user_data)->unref();
  };

  return true;
}

void FlutterMacOSGLCompositor::SetPresentCallback(
    const FlutterMacOSGLCompositor::PresentCallback& present_callback) {
  present_callback_ = present_callback;
}

}  // namespace flutter
