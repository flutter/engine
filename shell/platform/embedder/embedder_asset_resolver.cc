// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_asset_resolver.h"
#include "flutter/shell/platform/embedder/embedder_struct_macros.h"

namespace flutter {

class EmbedderAssetResolver final : public flutter::AssetResolver {
 public:
  explicit EmbedderAssetResolver(EmbedderAssetResolverGetAsset get_asset)
      : get_asset_(get_asset) {}

  ~EmbedderAssetResolver() = default;

  // |AssetResolver|
  bool IsValid() const override { return (bool)get_asset_; }

  // |AssetResolver|
  bool IsValidAfterAssetManagerChange() const override { return true; }

  // |AssetResolver|
  AssetResolverType GetType() const override {
    return AssetResolverType::kEmbedderAssetResolver;
  }

  // |AssetResolver|
  std::unique_ptr<fml::Mapping> GetAsMapping(
      const std::string& asset_name) const override {
    if (!get_asset_) {
      return nullptr;
    }

    return get_asset_(asset_name.c_str());
  }

 private:
  EmbedderAssetResolverGetAsset get_asset_;

  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderAssetResolver);
};

std::unique_ptr<flutter::AssetResolver> CreateEmbedderAssetResolver(
    EmbedderAssetResolverGetAsset get_asset) {
  return std::make_unique<EmbedderAssetResolver>(std::move(get_asset));
}

}  // namespace flutter
