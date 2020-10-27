// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/macos/framework/Source/FlutterMacOSCompositor.h"

#include "flutter/fml/logging.h"
#include "flutter/fml/platform/darwin/cf_utils.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/utils/mac/SkCGUtils.h"

namespace flutter {

FlutterMacOSCompositor::FlutterMacOSCompositor(SkISize surface_size,
                                               sk_sp<GrDirectContext> context,
                                               FlutterViewController* view_controller)
    : surface_size_(surface_size), context_(context), view_controller_(view_controller) {
  NSLog(@"\n init FlutterMacOSCompositor \n");
  FML_CHECK(!surface_size_.isEmpty()) << "Surface size must not be empty";
  FML_CHECK(context_);
}

FlutterMacOSCompositor::~FlutterMacOSCompositor() = default;

bool FlutterMacOSCompositor::CreateBackingStore(
    const FlutterBackingStoreConfig* config,
    FlutterBackingStore* backing_store_out) {

  FlutterOpenGLBackingStore open_gl_backing_store;
  switch (backing_store_out->type) {
    case kFlutterBackingStoreTypeOpenGL:
      NSLog(@"kFlutterBackingStoreTypeOpenGL");
      open_gl_backing_store = backing_store_out->open_gl;
      switch (open_gl_backing_store.type) {
        case kFlutterOpenGLTargetTypeFramebuffer:
          NSLog(@"kFlutterOpenGLTargetTypeFramebuffer");
          break;
        case kFlutterOpenGLTargetTypeTexture:
          NSLog(@"kFlutterOpenGLTargetTypeTexture");
          break;
      }
      break;
    case kFlutterBackingStoreTypeSoftware:
      NSLog(@"kFlutterBackingStoreTypeSoftware");
      break;
  } 

  // bool success = CreateSoftwareRenderSurface(config, backing_store_out);
  // bool success = CreateGLRenderSurface(config, backing_store_out);
  bool success = CreateFramebuffer(config, backing_store_out);

  return success;
}

bool FlutterMacOSCompositor::CollectBackingStore(
    const FlutterBackingStore* backing_store) {
  // We have already set the destruction callback for the various backing
  // stores. Our user_data is just the canvas from that backing store and does
  // not need to be explicitly collected. Embedders might have some other state
  // they want to collect though.
  return true;
}

bool FlutterMacOSCompositor::Present(const FlutterLayer** layers,
                                     size_t layers_count) {
  if (!UpdateOffscreenComposition(layers, layers_count)) {
    FML_LOG(ERROR)
        << "Could not update the off-screen composition in the MacOS compositor.";
    return false;
  }

  // for (size_t i = 0; i < layers_count; ++i) {
  //   const auto* layer = layers[i];
  //   SkSurface* surface;
  //   switch (layer->type) {
  //     case kFlutterLayerContentTypeBackingStore:
  //       surface = reinterpret_cast<SkSurface*>(layer->backing_store->user_data);
  //       // NSLog(@"surface == nullptr", surface == nullptr ? "")
  //       // surface->getCanvas()->flush();
  //       // present_callback_(layers, layers_count);
  //       break;
  //     default:
  //       NSLog(@"k");
  //   };
  // }

  // This is for presenting software layers.
  // for (size_t i = 0; i < layers_count; ++i) {
  //   const auto* layer = layers[i];

  //   sk_sp<SkImage> layer_image;
  //   SkPixmap pixmap;
  //   CGImageRef imageRef;

  //   switch (layer->type) {
  //     case kFlutterLayerContentTypeBackingStore:
  //       layer_image =
  //           reinterpret_cast<SkSurface*>(layer->backing_store->user_data)
  //               ->makeImageSnapshot();

  //       if (!layer_image->peekPixels(&pixmap)) {
  //           // Should raise an error here.
  //           NSLog(@"cannot peekPixels");
  //           return false;
  //       }

  //       imageRef =  CGImageCreate(pixmap.width(),     // width
  //                                 pixmap.height(),    // height
  //                                 8,                  // bits per component
  //                                 32,                 // bits per pixel
  //                                 pixmap.rowBytes(),  // bytes per row
  //                                 CGColorSpaceCreateDeviceRGB(),         // colorspace
  //                                 kCGImageAlphaPremultipliedLast,  // bitmap info
  //                                 CGDataProviderCreateWithData(
  //                                 nullptr,          // info
  //                                 pixmap.addr32(),  // data
  //                                 pixmap.computeByteSize(),      // size
  //                                 nullptr           // release callback
  //                                 ),      // data provider
  //                                 nullptr,                   // decode array
  //                                 false,                     // should interpolate
  //                                 kCGRenderingIntentDefault  // rendering intent
  //                                 );

  //       [view_controller_.view setWantsLayer:YES];
  //       view_controller_.view.layer.contents = (__bridge id) imageRef;

  //       break;
  //     case kFlutterLayerContentTypePlatformView:            
  //       NSLog(@"\nkFlutterLayerContentTypePlatformView\n");
  //       break;
  //   };
  // }

  return true;
}

bool FlutterMacOSCompositor::UpdateOffscreenComposition(
    const FlutterLayer** layers,
    size_t layers_count) {

  // Is this Clear canvas logic needed?
  const auto image_info = SkImageInfo::MakeN32Premul(surface_size_);

  // This is only for Software backing stores. 
  // auto surface = SkSurface::MakeRaster(image_info);

  // This is for GL Backing stores.
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

    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore:
        layer_image =
            reinterpret_cast<SkSurface*>(layer->backing_store->user_data)
                ->makeImageSnapshot();

        break;
      case kFlutterLayerContentTypePlatformView:
        canvas_offset = SkIPoint::Make(layer->offset.x, layer->offset.y);
        break;
    };

    // If the layer is not a platform view but the engine did not specify an
    // image for the backing store, it is an error.
    if (!layer_image && layer->type != kFlutterLayerContentTypePlatformView) {
      FML_LOG(ERROR) << "Could not snapshot layer in MacOS compositor: ";
                    //  << *layer;
      return false;
    }
  }

  return true;
}

bool FlutterMacOSCompositor::CreateSoftwareRenderSurface(
    const FlutterBackingStoreConfig* config,
    FlutterBackingStore* backing_store_out) {
  auto surface = SkSurface::MakeRaster(
      SkImageInfo::MakeN32Premul(config->size.width, config->size.height));

  if (!surface) {
    FML_LOG(ERROR)
        << "Could not create the render target for compositor layer.";
    return false;
  }

  SkPixmap pixmap;
  if (!surface->peekPixels(&pixmap)) {
    FML_LOG(ERROR) << "Could not peek pixels of pixmap.";
    return false;
  }

  backing_store_out->type = kFlutterBackingStoreTypeSoftware;
  backing_store_out->user_data = surface.get();
  backing_store_out->software.allocation = pixmap.addr();
  backing_store_out->software.row_bytes = pixmap.rowBytes();
  backing_store_out->software.height = pixmap.height();
  
  // The balancing unref is in the destruction callback.
  surface->ref();
  backing_store_out->software.user_data = surface.get();
  backing_store_out->software.destruction_callback = [](void* user_data) {
    reinterpret_cast<SkSurface*>(user_data)->unref();
  };

  return true;
}

bool FlutterMacOSCompositor::CreateGLRenderSurface(
    const FlutterBackingStoreConfig* config,
    FlutterBackingStore* backing_store_out
) {
  NSLog(@"CreateGLRenderSurface");
  const auto image_info =
      SkImageInfo::MakeN32Premul(config->size.width, config->size.height);

  auto surface = SkSurface::MakeRenderTarget(
      context_.get(),               // context
      SkBudgeted::kNo,              // budgeted
      image_info,                   // image info
      1,                            // sample count
      kBottomLeft_GrSurfaceOrigin,  // surface origin
      nullptr,                      // surface properties
      false                         // mipmaps
  );

  if (!surface) {
    FML_LOG(ERROR) << "Could not create render target for compositor layer.";
    return false;
  }

  GrBackendTexture render_texture = surface->getBackendTexture(
      SkSurface::BackendHandleAccess::kDiscardWrite_BackendHandleAccess);

  if (!render_texture.isValid()) {
    FML_LOG(ERROR) << "Backend render texture was invalid.";
    return false;
  }

  GrGLTextureInfo texture_info = {};
  if (!render_texture.getGLTextureInfo(&texture_info)) {
    FML_LOG(ERROR) << "Could not access backend texture info.";
    return false;
  }

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->user_data = surface.get();
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeTexture;
  backing_store_out->open_gl.texture.target = texture_info.fTarget;
  backing_store_out->open_gl.texture.name = texture_info.fID;
  backing_store_out->open_gl.texture.format = texture_info.fFormat;
  // The balancing unref is in the destruction callback.
  surface->ref();
  backing_store_out->open_gl.texture.user_data = surface.get();
  backing_store_out->open_gl.texture.destruction_callback =
      [](void* user_data) { reinterpret_cast<SkSurface*>(user_data)->unref(); };

  NSLog(@"CreateGLRenderSurface END");
  return true;
}

// This is from embedder_test_backingstore_producer.cc.
// bool FlutterMacOSCompositor::CreateFramebuffer(
//     const FlutterBackingStoreConfig* config,
//     FlutterBackingStore* backing_store_out) {
//   const auto image_info =
//       SkImageInfo::MakeN32Premul(config->size.width, config->size.height);

//   auto surface = SkSurface::MakeRenderTarget(
//       context_.get(),               // context
//       SkBudgeted::kNo,              // budgeted
//       image_info,                   // image info
//       1,                            // sample count
//       kBottomLeft_GrSurfaceOrigin,  // surface origin
//       nullptr,                      // surface properties
//       false                         // mipmaps
//   );

//   if (!surface) {
//     FML_LOG(ERROR) << "Could not create render target for compositor layer.";
//     return false;
//   }

//   GrBackendRenderTarget render_target = surface->getBackendRenderTarget(
//       SkSurface::BackendHandleAccess::kDiscardWrite_BackendHandleAccess);

//   if (!render_target.isValid()) {
//     FML_LOG(ERROR) << "Backend render target was invalid.";
//     return false;
//   }

//   GrGLFramebufferInfo framebuffer_info = {};
//   if (!render_target.getGLFramebufferInfo(&framebuffer_info)) {
//     FML_LOG(ERROR) << "Could not access backend framebuffer info.";
//     return false;
//   }

//   // NSLog(@"framebuffer_info.fFBOID: %u", framebuffer_info.fFBOID);
//   framebuffer_info.fFBOID = 0;

//   backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
//   backing_store_out->user_data = surface.get();
//   backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeFramebuffer;
//   backing_store_out->open_gl.framebuffer.target = framebuffer_info.fFormat;
//   backing_store_out->open_gl.framebuffer.name = framebuffer_info.fFBOID;
//   // The balancing unref is in the destruction callback.
//   surface->ref();
//   backing_store_out->open_gl.framebuffer.user_data = surface.get();
//   backing_store_out->open_gl.framebuffer.destruction_callback =
//       [](void* user_data) { reinterpret_cast<SkSurface*>(user_data)->unref(); };

//   return true;
// }

// This code is copied from gpu_surface_gl.cc / CreateOrUpdateSurfaces, WrapOnscreenSurface.
bool FlutterMacOSCompositor::CreateFramebuffer(
    const FlutterBackingStoreConfig* config,
    FlutterBackingStore* backing_store_out) {

  GrGLFramebufferInfo framebuffer_info = {};
  framebuffer_info.fFBOID = 0;
  framebuffer_info.fFormat = 0x8058;
  const SkColorType color_type = kRGBA_8888_SkColorType;


  GrBackendRenderTarget render_target(config->size.width,     // width
                                      config->size.height,    // height
                                      0,                // sample count
                                      0,                // stencil bits (TODO)
                                      framebuffer_info  // framebuffer info
  );

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

  backing_store_out->type = kFlutterBackingStoreTypeOpenGL;
  backing_store_out->user_data = surface.get();
  backing_store_out->open_gl.type = kFlutterOpenGLTargetTypeFramebuffer;
  backing_store_out->open_gl.framebuffer.target = framebuffer_info.fFormat;
  backing_store_out->open_gl.framebuffer.name = framebuffer_info.fFBOID;
  // The balancing unref is in the destruction callback.
  surface->ref();
  backing_store_out->open_gl.framebuffer.user_data = surface.get();
  backing_store_out->open_gl.framebuffer.destruction_callback =
      [](void* user_data) { reinterpret_cast<SkSurface*>(user_data)->unref(); };

  return true;
}


void FlutterMacOSCompositor::SetPresentCallback(const FlutterMacOSCompositor::PresentCallback& present_callback) {
  present_callback_ = present_callback;
}


} // namespace flutter
