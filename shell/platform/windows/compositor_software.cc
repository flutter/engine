// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/windows/compositor_software.h"

#include "flutter/shell/platform/windows/flutter_windows_view.h"

namespace flutter {

CompositorSoftware::CompositorSoftware(FlutterWindowsEngine* engine)
    : engine_(engine) {}

bool CompositorSoftware::CreateBackingStore(
    const FlutterBackingStoreConfig& config,
    FlutterBackingStore* result) {
  void* allocation = std::malloc(config.size.width * config.size.height * 4);
  if (!allocation) {
    return false;
  }

  result->type = kFlutterBackingStoreTypeSoftware;
  result->software.allocation = allocation;
  result->software.height = config.size.height;
  result->software.row_bytes = config.size.width * 4;
  result->software.destruction_callback = nullptr;
  result->software.user_data = nullptr;
  return true;
}

bool CompositorSoftware::CollectBackingStore(const FlutterBackingStore* store) {
  std::free(const_cast<void*>(store->software.allocation));
  return true;
}

bool CompositorSoftware::Present(const FlutterLayer** layers,
                                 size_t layers_count) {
  FML_DCHECK(layers_count == 1);
  FML_DCHECK(layers[0]->type == kFlutterLayerContentTypeBackingStore);
  FML_DCHECK(layers[0]->backing_store->type ==
             kFlutterBackingStoreTypeSoftware);

  const auto& backing_store = layers[0]->backing_store->software;

  return engine_->view()->PresentSoftwareBitmap(
      backing_store.allocation, backing_store.row_bytes, backing_store.height);
}

}  // namespace flutter
