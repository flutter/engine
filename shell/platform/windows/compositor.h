// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_WINDOWS_COMPOSITOR_H_
#define FLUTTER_SHELL_PLATFORM_WINDOWS_COMPOSITOR_H_

#include "flutter/shell/platform/embedder/embedder.h"

#include <memory>

#include "flutter/shell/platform/windows/platform_view_manager.h"

namespace flutter {

// Enables the Flutter engine to render content on Windows.
//
// The engine uses this to:
//
// 1. Create backing stores used for rendering Flutter content
// 2. Composite and present Flutter content and platform views onto a view
//
// Platform views are not yet supported.
class Compositor {
 public:
  Compositor(std::unique_ptr<PlatformViewManager> manager) : platform_view_manager_(std::move(manager)) {}
  virtual ~Compositor() = default;

  // Creates a backing store used for rendering Flutter content.
  //
  // The backing store's configuration is stored in |backing_store_out|.
  virtual bool CreateBackingStore(const FlutterBackingStoreConfig& config,
                                  FlutterBackingStore* backing_store_out) = 0;

  // Destroys a backing store and releases its resources.
  virtual bool CollectBackingStore(const FlutterBackingStore* store) = 0;

  // Present Flutter content and platform views onto the view.
  virtual bool Present(const FlutterLayer** layers, size_t layers_count) = 0;

 protected:
  std::unique_ptr<PlatformViewManager> platform_view_manager_;
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_WINDOWS_COMPOSITOR_H_
