// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTER_COMPOSITORIOS_H_
#define SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTER_COMPOSITORIOS_H_

#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterPlatformViews_Internal.h"
#import "flutter/shell/platform/darwin/ios/framework/Source/FlutterView.h"
#include "flutter/shell/platform/embedder/embedder.h"
#import "fml/platform/darwin/scoped_nsobject.h"

namespace flutter {

// FlutterCompositorIOS creates and manages the backing stores used for
// rendering Flutter content and presents Flutter content and Platform views.
class FlutterCompositorIOS {
 public:
  explicit FlutterCompositorIOS(
      std::shared_ptr<flutter::FlutterPlatformViewsController>
          platform_views_controller);

  ~FlutterCompositorIOS() = default;

  bool CreateBackingStore(const FlutterBackingStoreConfig* config,
                          FlutterBackingStore* backing_store_out);

  // Presents the FlutterLayers by updating the FlutterView specified by
  // `view_id` using the layer content. Sets frame_started_ to false.
  bool Present(uint64_t view_id,
               const FlutterLayer** layers,
               size_t layers_count);

 private:
  std::shared_ptr<flutter::FlutterPlatformViewsController>
      platform_views_controller_;

  void PresentPlatformViews(FlutterView* default_base_view,
                            const FlutterLayer** layers,
                            size_t layers_count);

  void PresentPlatformView(FlutterView* default_base_view,
                           const FlutterLayer* layer,
                           size_t layer_position);

  FML_DISALLOW_COPY_AND_ASSIGN(FlutterCompositorIOS);
};

}  // namespace flutter

#endif  // SHELL_PLATFORM_IOS_FRAMEWORK_SOURCE_FLUTTER_COMPOSITORIOS_H_
