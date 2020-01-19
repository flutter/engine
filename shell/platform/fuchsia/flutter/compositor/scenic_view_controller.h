// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSDstyle license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_COMPOSITOR_SCENIC_VIEW_CONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_COMPOSITOR_SCENIC_VIEW_CONTROLLER_H_

#include <fuchsia/ui/gfx/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/ui/scenic/cpp/id.h>
#include <lib/ui/scenic/cpp/resources.h>
#include <lib/ui/scenic/cpp/view_ref_pair.h>

#include "flutter/fml/macros.h"
#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/compositor/scenic_session.h"

namespace flutter_runner {

// The embedder component that is responsible for manipulating the embedders'
// Scenic |View| in response to new frames being created by the renderer.
//
// This component exists on the GPU thread.
class ScenicViewController final {
 public:
  ScenicViewController(ScenicSession& session,
                       fuchsia::ui::views::ViewToken view_token);
  ~ScenicViewController();

  fuchsia::ui::views::ViewRef view_ref_copy() const;

  // Methods invoked by the Flutter engine via |FlutterCompositor| callbacks.
  // Called on the GPU thread.
  bool CreateBackingStore(const FlutterBackingStoreConfig* layer_config,
                          FlutterBackingStore* backing_store_out);
  bool CollectBackingStore(const FlutterBackingStore* backing_store);
  bool PresentLayers(const FlutterLayer** layers, size_t layer_count);

  // Methods invoked by the callbacks from |ScenicInputHandler|.
  void OnMetricsChanged(fuchsia::ui::gfx::Metrics new_metrics);
  void OnChildViewConnected(scenic::ResourceId view_id);
  void OnChildViewDisconnected(scenic::ResourceId view_id);
  void OnChildViewStateChanged(scenic::ResourceId view_id, bool is_rendering);
  void OnEnableWireframe(bool enable);

 private:
  ScenicViewController(ScenicSession& session,
                       fuchsia::ui::views::ViewToken view_token,
                       scenic::ViewRefPair view_ref_pair);

  ScenicSession& session_;

  fuchsia::ui::views::ViewRef root_view_ref_;
  scenic::View root_view_;
  scenic::EntityNode root_node_;

  FML_DISALLOW_COPY_AND_ASSIGN(ScenicViewController);
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_COMPOSITOR_SCENIC_VIEW_CONTROLLER_H_
