// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "flutter/shell/platform/darwin/ios/ios_surface.h"

#import "flutter/shell/platform/darwin/ios/ios_surface_gl.h"
#import "flutter/shell/platform/darwin/ios/ios_surface_software.h"

#include "flutter/shell/platform/darwin/graphics/rendering_api_selection.h"

#if FLUTTER_SHELL_ENABLE_METAL
#import "flutter/shell/platform/darwin/ios/ios_surface_metal.h"
#endif  // FLUTTER_SHELL_ENABLE_METAL

namespace flutter {

std::unique_ptr<IOSSurface> IOSSurface::Create(const IOSContext& context,
                                               fml::scoped_nsobject<CALayer> layer) {
  FML_DCHECK(layer);

  if ([layer.get() isKindOfClass:[CAEAGLLayer class]]) {
    return std::make_unique<IOSSurfaceGL>(
        fml::scoped_nsobject<CAEAGLLayer>(
            reinterpret_cast<CAEAGLLayer*>([layer.get() retain])),  // EAGL layer
        std::move(context)                                          // context
    );
  }

#if FLUTTER_SHELL_ENABLE_METAL
  if (@available(iOS METAL_IOS_VERSION_BASELINE, *)) {
    if ([layer.get() isKindOfClass:[CAMetalLayer class]]) {
      return std::make_unique<IOSSurfaceMetal>(
          fml::scoped_nsobject<CAMetalLayer>(
              reinterpret_cast<CAMetalLayer*>([layer.get() retain])),  // Metal layer
          std::move(context)                                           // context
      );
    }
  }
#endif  // FLUTTER_SHELL_ENABLE_METAL

  return std::make_unique<IOSSurfaceSoftware>(std::move(layer),   // layer
                                              std::move(context)  // context
  );
}

IOSSurface::IOSSurface(const IOSContext& ios_context) : ios_context_(ios_context) {}

IOSSurface::~IOSSurface() = default;

}  // namespace flutter
