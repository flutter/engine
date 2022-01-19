// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_ASSET_RESOLVER_H_
#define FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_ASSET_RESOLVER_H_

#include <functional>

#include "flutter/assets/asset_resolver.h"
#include "flutter/shell/platform/embedder/embedder.h"

namespace flutter {

using EmbedderAssetResolverGetAsset = std::function<std::unique_ptr<fml::Mapping>(const char* /* asset_name */)>;

std::unique_ptr<flutter::AssetResolver> CreateEmbedderAssetResolver(
    EmbedderAssetResolverGetAsset get_asset);

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_EMBEDDER_EMBEDDER_ASSET_RESOLVER_H_
