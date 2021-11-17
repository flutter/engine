// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_ASSET_RESOLVER_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_ASSET_RESOLVER_H_

#include "flutter/assets/asset_resolver.h"
#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

std::unique_ptr<flutter::AssetResolver> CreateEmbedderAssetResolver(const FlutterEngineAssetResolver* resolver);

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_ASSET_RESOLVER_H_
