// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

class FlutterMacOSGLCompositor {
 public:
  FlutterMacOSGLCompositor(FlutterViewController* view_controller);

  virtual ~FlutterMacOSGLCompositor();

  bool CreateBackingStore(const FlutterBackingStoreConfig* config,
                          FlutterBackingStore* backing_store_out);

  bool CollectBackingStore(const FlutterBackingStore* backing_store);

  bool Present(const FlutterLayer** layers, size_t layers_count);

  using PresentCallback = std::function<bool()>;

  void SetPresentCallback(const PresentCallback& present_callback);

 protected:
  sk_sp<GrDirectContext> context_;
  FlutterViewController* view_controller_;
  PresentCallback present_callback_;

  // Global counter for testing.
  int count_ = 0;

  // For FBO testing.
  bool fbo_created_ = false;
  GrGLFramebufferInfo fbo_;
  

  bool CreateSoftwareRenderSurface(const FlutterBackingStoreConfig* config,
                                   FlutterBackingStore* renderer_out);

  bool CreateGLRenderSurface(const FlutterBackingStoreConfig* config,
                             FlutterBackingStore* backing_store_out);

  bool CreateFramebuffer(const FlutterBackingStoreConfig* config,
                             FlutterBackingStore* backing_store_out);

  FML_DISALLOW_COPY_AND_ASSIGN(FlutterMacOSGLCompositor);
};

}  // namespace flutter
