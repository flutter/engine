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
  printf("\n init FlutterMacOSCompositor \n");
  FML_CHECK(!surface_size_.isEmpty()) << "Surface size must not be empty";
  FML_CHECK(context_);
}

FlutterMacOSCompositor::~FlutterMacOSCompositor() = default;

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

bool FlutterMacOSCompositor::CreateBackingStore(
    const FlutterBackingStoreConfig* config,
    FlutterBackingStore* backing_store_out) {
  bool success = CreateSoftwareRenderSurface(config, backing_store_out);

  SkSurface* backing_store = reinterpret_cast<SkSurface*>(backing_store_out->user_data);

  SkPixmap pixmap;
  if (!backing_store->peekPixels(&pixmap)) {
    return false;
  }

  // Some basic sanity checking.
  uint64_t expected_pixmap_data_size = pixmap.width() * pixmap.height() * 4;

  const size_t pixmap_size = pixmap.computeByteSize();

  if (expected_pixmap_data_size != pixmap_size) {
    return false;
  }

  fml::CFRef<CGColorSpaceRef> colorspace(CGColorSpaceCreateDeviceRGB());

  // Setup the data provider that gives CG a view into the pixmap.
  fml::CFRef<CGDataProviderRef> pixmap_data_provider(CGDataProviderCreateWithData(
      nullptr,          // info
      pixmap.addr32(),  // data
      pixmap_size,      // size
      nullptr           // release callback
      ));

  if (!pixmap_data_provider) {
    return false;
  }

  // Create the CGImageRef representation on the pixmap.
  fml::CFRef<CGImageRef> pixmap_image(CGImageCreate(pixmap.width(),     // width
                                                    pixmap.height(),    // height
                                                    8,                  // bits per component
                                                    32,                 // bits per pixel
                                                    pixmap.rowBytes(),  // bytes per row
                                                    colorspace,         // colorspace
                                                    kCGImageAlphaPremultipliedLast,  // bitmap info
                                                    pixmap_data_provider,      // data provider
                                                    nullptr,                   // decode array
                                                    false,                     // should interpolate
                                                    kCGRenderingIntentDefault  // rendering intent
                                                    ));

  if (!pixmap_image) {
    return false;
  }

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

  for (size_t i = 0; i < layers_count; ++i) {
    const auto* layer = layers[i];

    sk_sp<SkImage> layer_image;
    SkPixmap pixmap;
    CGImageRef imageRef;

    switch (layer->type) {
      case kFlutterLayerContentTypeBackingStore:
        layer_image =
            reinterpret_cast<SkSurface*>(layer->backing_store->user_data)
                ->makeImageSnapshot();

        if (!layer_image->peekPixels(&pixmap)) {
            // Should raise an error here.
            printf("cannot peekPixels");
        }

        imageRef =  CGImageCreate(pixmap.width(),     // width
                                  pixmap.height(),    // height
                                  8,                  // bits per component
                                  32,                 // bits per pixel
                                  pixmap.rowBytes(),  // bytes per row
                                  CGColorSpaceCreateDeviceRGB(),         // colorspace
                                  kCGImageAlphaPremultipliedLast,  // bitmap info
                                  CGDataProviderCreateWithData(
                                  nullptr,          // info
                                  pixmap.addr32(),  // data
                                  pixmap.computeByteSize(),      // size
                                  nullptr           // release callback
                                  ),      // data provider
                                  nullptr,                   // decode array
                                  false,                     // should interpolate
                                  kCGRenderingIntentDefault  // rendering intent
                                  );

        [view_controller_.view setWantsLayer:YES];
        view_controller_.view.layer.contents = (__bridge id) imageRef;

        break;
      case kFlutterLayerContentTypePlatformView:            
        printf("\nkFlutterLayerContentTypePlatformView\n");
        break;
    };
  }

  return true;
}

bool FlutterMacOSCompositor::UpdateOffscreenComposition(
    const FlutterLayer** layers,
    size_t layers_count) {

  const auto image_info = SkImageInfo::MakeN32Premul(surface_size_);

  // This is only for Software backing stores. 
  auto surface = SkSurface::MakeRaster(image_info);

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

} // namespace flutter
