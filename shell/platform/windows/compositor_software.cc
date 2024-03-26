// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/compositor_software.h"

#include "flutter/shell/platform/windows/flutter_windows_engine.h"
#include "flutter/shell/platform/windows/flutter_windows_view.h"

namespace flutter {

CompositorSoftware::CompositorSoftware(FlutterWindowsEngine* engine)
    : engine_(engine) {}

bool CompositorSoftware::CreateBackingStore(
    const FlutterBackingStoreConfig& config,
    FlutterBackingStore* result) {
  size_t size = config.size.width * config.size.height * 4;
  void* allocation = std::calloc(size, sizeof(uint8_t));
  if (!allocation) {
    return false;
  }

  result->type = kFlutterBackingStoreTypeSoftware;
  result->software.allocation = allocation;
  result->software.height = config.size.height;
  result->software.row_bytes = config.size.width * 4;
  result->software.user_data = nullptr;
  result->software.destruction_callback = [](void* user_data) {
    // Backing store destroyed in `CompositorSoftware::CollectBackingStore`, set
    // on FlutterCompositor.collect_backing_store_callback during engine start.
  };
  return true;
}

bool CompositorSoftware::CollectBackingStore(const FlutterBackingStore* store) {
  std::free(const_cast<void*>(store->software.allocation));
  return true;
}

bool CompositorSoftware::Present(FlutterViewId view_id,
                                 const FlutterLayer** layers,
                                 size_t layers_count) {
  FlutterWindowsView* view = engine_->view(view_id);
  if (!view) {
    return false;
  }

  // Clear the view if there are no layers to present.
  if (layers_count == 0) {
    return view->ClearSoftwareBitmap();
  }

  // TODO: Support compositing layers and platform views.
  // See: https://github.com/flutter/flutter/issues/31713
  //FML_DCHECK(layers_count == 1);
  //FML_DCHECK(layers[0]->offset.x == 0 && layers[0]->offset.y == 0);
  //FML_DCHECK(layers[0]->type == kFlutterLayerContentTypeBackingStore);
  //FML_DCHECK(layers[0]->backing_store->type ==
  //           kFlutterBackingStoreTypeSoftware);

  HWND hwnd = view->GetWindowHandle();
  RECT rect;
  GetClientRect(hwnd, &rect);
  int width = layers[0]->size.width;//rect.right - rect.left;
  int height = layers[0]->size.height;//rect.bottom - rect.top;
  std::vector<uint32_t> allocation(width * height);

  for (const FlutterLayer** layer = layers; layer < layers + layers_count; layer++) {
    // TODO(schectman): handle platform view type layers.
    // https://github.com/flutter/flutter/issues/143375
    FML_DCHECK((*layer)->type == kFlutterLayerContentTypeBackingStore);
    auto& backing_store = *(*layer)->backing_store;
    FML_DCHECK(backing_store.type == kFlutterBackingStoreTypeSoftware);
    auto src_data = static_cast<const uint32_t*>(backing_store.software.allocation);
    auto& offset = (*layer)->offset;
    auto& size = (*layer)->size;

    for (int y_src = 0; y_src < size.height; y_src++) {
      int y_dst = y_src + offset.y;
      if (y_dst < 0) {
        continue;
      }
      if (y_dst >= height) {
        break;
      }
      for (int x_src = 0; x_src < size.width; x_src++) {
        int x_dst = x_src + offset.x;
        if (x_dst < 0) {
          continue;
        }
        if (x_dst >= width) {
          break;
        }
        size_t i_src = y_src * size.width + x_src;
        size_t i_dst = y_dst * width + x_dst;
        uint32_t src = src_data[i_src];
        uint32_t dst = allocation[i_dst];

        int r_src = (src >> 0) & 0xff;
        int g_src = (src >> 8) & 0xff;
        int b_src = (src >> 16) & 0xff;
        int a_src = (src >> 24) & 0xff;

        int r_dst = (dst >> 0) & 0xff;
        int g_dst = (dst >> 8) & 0xff;
        int b_dst = (dst >> 16) & 0xff;

        int r = (r_dst * 255 + (r_src - r_dst) * a_src) / 255;
        int g = (g_dst * 255 + (g_src - g_dst) * a_src) / 255;
        int b = (b_dst * 255 + (b_src - b_dst) * a_src) / 255;

        allocation[i_dst] = (r << 0) | (g << 8) | (b << 16) | (0xff << 24);
      }
    }
  }

  //const auto& backing_store = layers[0]->backing_store->software;

  //return view->PresentSoftwareBitmap(
  //    backing_store.allocation, backing_store.row_bytes, backing_store.height);

  return view->PresentSoftwareBitmap(static_cast<void*>(allocation.data()), width * 4, height);
}

}  // namespace flutter
