// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSDstyle license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPOSITOR_SCENIC_WINDOW_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPOSITOR_SCENIC_WINDOW_H_

#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/sys/cpp/service_directory.h>

#include "flutter/shell/platform/embedder/embedder.h"
// #include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_input_handler.h"
// #include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_session.h"
// #include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_view_controller.h"

namespace flutter_runner {

// This embedder component is responsible for interacting with the Scenic
// compositor on Fuchsia.  It encapsulates all of the state required to manage
// a single Scenic |Session| and |View|.
//
// The internal structure of the |ScenicWindow| is broken into 3 general parts:
//  + |ScenicSession| manages the Session lifetime and the timing of when frames
//    are presented to the compositor.
//  + |ScenicViewController| manages the various Scenic resources that comprise
//    the rendered scene.  It operates on the GPU thread.
//  + |ScenicInputHandler| manages the |SessionListener|, which allows it to
//    respond to various Scenic events for input and rendering.  It operates on
//    the platform thread.  Some of the events handled by the this object are
//    related to the rendered scene.  Those are dispatched to the GPU thread
//    for processing by the |ScenicViewController|.  As a consequence of
//    handling input events, this object is also responsible for managing IME
//    and a11y state for its bound instance of the Flutter engine.
class ScenicWindow final {
 public:
  using ErrorCallback = std::function<void(ScenicWindow*)>;

  ScenicWindow(const std::string& debug_label,
               ErrorCallback error_callback,
               fuchsia::ui::views::ViewToken view_token,
               std::shared_ptr<sys::ServiceDirectory> incoming_services);
  ~ScenicWindow();
  ScenicWindow(const ScenicWindow&) = delete;
  ScenicWindow& operator=(const ScenicWindow&) = delete;

  // This method is invoked by the embedder to bind the |ScenicWindow| to a
  // |FlutterEngine| instance.
  void BindFlutterEngine(FlutterEngine engine) { flutter_engine_ = engine; }

  // These methods are invoked by the Flutter engine via the Embedder API; they
  // are called on the UI thread.
  void AwaitPresent(intptr_t baton);

  // These methods are invoked by the Flutter engine via the Embedder API; they
  // are called on the GPU thread.
  bool CreateBackingStore(const FlutterBackingStoreConfig* layer_config,
                          FlutterBackingStore* backing_store_out);
  bool CollectBackingStore(const FlutterBackingStore* backing_store);
  bool PresentLayers(const FlutterLayer** layers, size_t layer_count);

  // These methods are invoked by the Flutter engine via the Embedder API; they
  // are called on the platform thread.
  void PlatformMessageResponse(const FlutterPlatformMessage* message);
  void UpdateSemanticsNode(const FlutterSemanticsNode* node);
  void UpdateSemanticsCustomAction(const FlutterSemanticsCustomAction* action);

 private:
  // ScenicSession session_;
  // ScenicViewController view_controller_;
  // ScenicInputHandler input_handler_;

  FlutterEngine flutter_engine_ = nullptr;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_FLUTTER_COMPOSITOR_SCENIC_WINDOW_H_
