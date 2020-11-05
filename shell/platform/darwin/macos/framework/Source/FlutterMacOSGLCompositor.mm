// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMacOSGLCompositor.h"

#import <OpenGL/gl.h>
#import "flutter/fml/logging.h"
#import "flutter/fml/platform/darwin/cf_utils.h"
#import "third_party/skia/include/core/SkCanvas.h"
#import "third_party/skia/include/core/SkSurface.h"
#import "third_party/skia/include/gpu/gl/GrGLAssembleInterface.h"
#import "third_party/skia/include/utils/mac/SkCGUtils.h"

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
  return CreateFramebuffer(config, backing_store_out);
}

bool FlutterMacOSGLCompositor::CollectBackingStore(const FlutterBackingStore* backing_store) {
  // We have already set the destruction callback for the various backing
  // stores. Our user_data is just the canvas from that backing store and does
  // not need to be explicitly collected. Embedders might have some other state
  // they want to collect though.
  return true;
}

bool FlutterMacOSGLCompositor::Present(const FlutterLayer** layers, size_t layers_count) {
  if (!UpdateOffscreenComposition(layers, layers_count)) {
    NSLog(@"failed to update offscreen composition");
    return false;
  }

  bool success = true;
  SkCanvas* canvas;
  GLuint fbo_id;
  GLuint dest_fbo_id;

  CGSize size;
  for (size_t i = 0; i < layers_count; ++i) {
    const auto* layer = layers[i];
    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore:
        if (i != 0) continue;
        fbo_id = const_cast<FlutterBackingStore*>(layer->backing_store)->open_gl.framebuffer.name;
        canvas = reinterpret_cast<SkSurface*>(layer->backing_store->user_data)
            ->getCanvas();
        canvas->flush();
        glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_id);
        size = CGSizeMake(layer->size.width, layer->size.height);
        dest_fbo_id = [view_controller_.flutterView getFrameBufferIdForSize:size];
        NSLog(@"dest_fbo_id %u", dest_fbo_id);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dest_fbo_id);  // if not already bound
        glBlitFramebuffer(0, 0, layer->size.width, layer->size.height, 0, 0, layer->size.width, layer->size.height,
                          GL_COLOR_BUFFER_BIT, GL_NEAREST);
        break;
      case kFlutterLayerContentTypePlatformView:
        NSLog(@"Present backing store");
        [view_controller_.view addSubview: view_controller_.view_map.at(1)];
        break;
    };
  }
  success = success && present_callback_();
  return success;
}

bool FlutterMacOSGLCompositor::CreateFramebuffer(const FlutterBackingStoreConfig* config,
                                               FlutterBackingStore* backing_store_out) {
  NSLog(@"CreateFramebuffer");
  GLuint texture;
  GrGLFramebufferInfo framebuffer_info = {};
  glGenFramebuffersEXT(1, &framebuffer_info.fFBOID);
  NSLog(@"framebuffer id %u:", framebuffer_info.fFBOID);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer_info.fFBOID);
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, config->size.width, config->size.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                  GL_TEXTURE_2D, texture, 0);
                  
  if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
    NSLog(@"invalid framebuffer");
    return false;
  }

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

bool FlutterMacOSGLCompositor::UpdateOffscreenComposition(
    const FlutterLayer** layers,
    size_t layers_count) {
  const auto image_info = SkImageInfo::MakeN32Premul(600,600);

  auto surface =
      SkSurface::MakeRenderTarget(context_.get(),            // context
                                  SkBudgeted::kNo,           // budgeted
                                  image_info,                // image info
                                  1,                         // sample count
                                  kTopLeft_GrSurfaceOrigin,  // surface origin
                                  nullptr,  // surface properties
                                  false     // create mipmaps
      );

  if (!surface) {
    FML_LOG(ERROR) << "Could not update the off-screen composition.";
    return false;
  }

  auto canvas = surface->getCanvas();

  // This has to be transparent because we are going to be compositing this
  // sub-hierarchy onto the on-screen surface.
  canvas->clear(SK_ColorTRANSPARENT);

  for (size_t i = 0; i < layers_count; ++i) {
    const auto* layer = layers[i];

    sk_sp<SkImage> platform_renderered_contents;

    sk_sp<SkImage> layer_image;
    SkIPoint canvas_offset = SkIPoint::Make(0, 0);
    FlutterBackingStore* backing_store;
    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore:
        backing_store = const_cast<FlutterBackingStore*>(layer->backing_store);
        // glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, backing_store->open_gl.framebuffer.name);
        layer_image =
            reinterpret_cast<SkSurface*>(layer->backing_store->user_data)
                ->makeImageSnapshot();

        break;
      case kFlutterLayerContentTypePlatformView:
        // layer_image =
        //     platform_view_renderer_callback_
        //         ? platform_view_renderer_callback_(*layer, context_.get())
        //         : nullptr;
        // canvas_offset = SkIPoint::Make(layer->offset.x, layer->offset.y);
        NSLog(@"kFlutterLayerContentTypePlatformView");
        break;
    };

    // If the layer is not a platform view but the engine did not specify an
    // image for the backing store, it is an error.
    if (!layer_image && layer->type != kFlutterLayerContentTypePlatformView) {
      FML_LOG(ERROR) << "Could not snapshot layer in test compositor: ";
                    //  << *layer;
      return false;
    }

    // The test could have just specified no contents to be rendered in place of
    // a platform view. This is not an error.
    if (layer_image) {
      // The image rendered by Flutter already has the correct offset and
      // transformation applied. The layers offset is meant for the platform.
      canvas->drawImage(layer_image.get(), canvas_offset.x(),
                        canvas_offset.y());
    }
  }

  return true;
}

}  // namespace flutter
