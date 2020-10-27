// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterSurfaceManager.h"
#include "flutter/shell/platform/darwin/macos/framework/Source/FlutterViewController_Internal.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"

namespace flutter {

/**
 * FlutterMacOSGLCompositor creates and manages backing stores used for
 * rendering Flutter content and presents Flutter content and Platform views.
 */
class FlutterMacOSGLCompositor {
 public:
  FlutterMacOSGLCompositor(FlutterViewController* view_controller,
                           NSOpenGLContext* open_gl_context);

  virtual ~FlutterMacOSGLCompositor();

  // Creates a backing store according to FlutterBackingStoreConfig
  // by modifying backing_store_out.
  bool CreateBackingStore(const FlutterBackingStoreConfig* config,
                          FlutterBackingStore* backing_store_out);

  // Releases the memory for any state used by the backing store.
  bool CollectBackingStore(const FlutterBackingStore* backing_store);

  // Presents the FlutterLayers by updating FlutterView(s) using the
  // layer content.
  bool Present(const FlutterLayer** layers, size_t layers_count);

  using PresentCallback = std::function<bool()>;

  // PresentCallback is called at the end of the Present function.
  void SetPresentCallback(const PresentCallback& present_callback);

 protected:
  FlutterViewController* view_controller_;
  PresentCallback present_callback_;
  NSOpenGLContext* open_gl_context_;

  // Creates a FlutterSurfaceManager and uses the FlutterSurfaceManager's
  // underlying FBO and texture in the backing store.
  bool CreateBackingStoreUsingSurfaceManager(
      const FlutterBackingStoreConfig* config,
      FlutterBackingStore* backing_store_out);

  FML_DISALLOW_COPY_AND_ASSIGN(FlutterMacOSGLCompositor);
};

}  // namespace flutter
