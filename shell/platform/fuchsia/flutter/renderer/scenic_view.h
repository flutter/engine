// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_VIEW_H_
#define FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_VIEW_H_

#include <fuchsia/ui/scenic/cpp/fidl.h>
#include <fuchsia/ui/views/cpp/fidl.h>
#include <lib/ui/scenic/cpp/id.h>
#include <lib/ui/scenic/cpp/resources.h>
#include <lib/ui/scenic/cpp/view_ref_pair.h>
#include <lib/zx/event.h>

#include <memory>
#include <optional>
#include <vector>

#include "flutter/shell/platform/embedder/embedder.h"
#include "flutter/shell/platform/fuchsia/flutter/renderer/scenic_session.h"

namespace flutter_runner {

// The embedder component that is responsible for manipulating the embedders'
// Scenic |View| in response to new frames being created by the renderer.
//
// This component exists on the raster thread.
class ScenicView final {
 public:
  ScenicView(const std::string& debug_label,
             ScenicSession& session,
             fuchsia::ui::views::ViewToken view_token,
             scenic::ViewRefPair view_ref_pair);
  ScenicView(const ScenicView&) = delete;
  ScenicView(ScenicView&&) = delete;
  ~ScenicView() = default;

  ScenicView& operator=(const ScenicView&) = delete;
  ScenicView& operator=(ScenicView&&) = delete;

  // The Flutter engine invokes these methods via |FlutterCompositor| callbacks
  // on the raster thread.
  bool CreateBackingStore(const FlutterBackingStoreConfig* layer_config,
                          FlutterBackingStore* backing_store_out);
  bool CollectBackingStore(const FlutterBackingStore* backing_store);
  bool PresentLayers(const FlutterLayer** layers, size_t layer_count);
  bool IsBackingStoreAvailable(const FlutterBackingStore* backing_store);

  // |ScenicPlatformBridge| invokes these callbacks on the raster thread.
  void OnChildViewConnected(scenic::ResourceId view_id);
  void OnChildViewDisconnected(scenic::ResourceId view_id);
  void OnChildViewStateChanged(scenic::ResourceId view_id, bool is_rendering);
  void OnPixelRatioChanged(float pixel_ratio);
  void OnEnableWireframe(bool enable);

 private:
  struct BackingStore {
    // TODO(SCN-268): remove unique_ptr when |Memory| and |Image| are moveable
    std::unique_ptr<scenic::Memory> memory;
    std::unique_ptr<scenic::Image> image;

    const void* base_address = nullptr;

    uint32_t width = 0;
    uint32_t stride = 0;
    uint32_t height = 0;

    uint32_t age = 0;
  };

  struct Layer {
    scenic::Material material;
    scenic::ShapeNode shape;
  };

  struct EmbeddedView {
    scenic::OpacityNodeHACK opacity;
    scenic::ViewHolder view_holder;
  };

  ScenicSession& session_;

  scenic::View root_view_;
  scenic::EntityNode root_node_;
  scenic::EntityNode scale_node_;

  std::vector<std::optional<BackingStore>> backing_stores_;
  std::vector<Layer> layers_;
  std::vector<EmbeddedView> embedded_views_;
};

}  // namespace flutter_runner

#endif  // FLUTTER_SHELL_PLATFORM_FUCHSIA_RENDERER_SCENIC_VIEW_H_
