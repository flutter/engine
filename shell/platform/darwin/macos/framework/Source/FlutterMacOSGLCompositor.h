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

// FlutterMacOSGLCompositor creates and manages the backing stores used for
// rendering Flutter content and presents Flutter content and Platform views.
// Platform views are not yet supported.
// FlutterMacOSGLCompositor is created and destroyed by FlutterEngine.
class FlutterMacOSGLCompositor {
 public:
  FlutterMacOSGLCompositor(FlutterViewController* view_controller);

  // Creates a FlutterSurfaceManager and uses the FlutterSurfaceManager's
  // underlying FBO and texture in the backing store.
  // Any additional state allocated for the backing store and
  // saved as user_data in the backing store must be collected
  // in the backing_store's desctruction_callback field which will
  // be called when the embedder collects the backing store.
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

 private:
  FlutterViewController* view_controller_;
  PresentCallback present_callback_;
  NSOpenGLContext* open_gl_context_;

  FML_DISALLOW_COPY_AND_ASSIGN(FlutterMacOSGLCompositor);
};

}  // namespace flutter
