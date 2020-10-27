// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

class FlutterMacOSCompositor {
 public:
  FlutterMacOSCompositor(SkISize surface_size, sk_sp<GrDirectContext> context, FlutterViewController* view_controller);

  virtual ~FlutterMacOSCompositor();

  bool CreateBackingStore(const FlutterBackingStoreConfig* config,
                          FlutterBackingStore* backing_store_out);

  bool CollectBackingStore(const FlutterBackingStore* backing_store);

  bool Present(const FlutterLayer** layers, size_t layers_count);

 protected:
  const SkISize surface_size_;
  sk_sp<GrDirectContext> context_;
  FlutterViewController* view_controller_;

  bool UpdateOffscreenComposition(const FlutterLayer** layers,
                                          size_t layers_count);

  bool CreateSoftwareRenderSurface(const FlutterBackingStoreConfig* config,
                                   FlutterBackingStore* renderer_out);

  FML_DISALLOW_COPY_AND_ASSIGN(FlutterMacOSCompositor);
};

}  // namespace flutter
