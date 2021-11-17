// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/shell/platform/embedder/embedder_asset_resolver.h"
#include "flutter/shell/platform/embedder/embedder_struct_macros.h"

namespace flutter {

class EmbedderAssetResolver final : public flutter::AssetResolver {
  public:
  explicit EmbedderAssetResolver(void* user_data,
                                 FlutterAssetResolverGetAssetCallback get_asset)
      : user_data_(user_data),
        get_asset_(get_asset) {}
  
  ~EmbedderAssetResolver() override {}

  // |AssetResolver|
  bool IsValid() const override {
    return true;
  }

  // |AssetResolver|
  bool IsValidAfterAssetManagerChange() const override {
    return true;
  }

  // |AssetResolver|
  AssetResolverType GetType() const override {
    return AssetResolverType::kEmbedderAssetResolver;
  }

  // |AssetResolver|
  std::unique_ptr<fml::Mapping> GetAsMapping(const std::string& asset_name) const override {
    if (!get_asset_) {
      return std::unique_ptr<fml::Mapping>();
    }

    FlutterEngineMapping mapping = get_asset_(asset_name.c_str(), user_data_);
    return std::unique_ptr<fml::Mapping>(reinterpret_cast<fml::Mapping*>(mapping));
  }

private:
  void* user_data_;
  FlutterAssetResolverGetAssetCallback get_asset_;
  
  FML_DISALLOW_COPY_AND_ASSIGN(EmbedderAssetResolver);
};

std::unique_ptr<flutter::AssetResolver> CreateEmbedderAssetResolver(const FlutterEngineAssetResolver* resolver) {
  return std::make_unique<EmbedderAssetResolver>(
    SAFE_ACCESS(resolver, user_data, nullptr),
    SAFE_ACCESS(resolver, get_asset, nullptr)
  );
}

}  // namespace flutter
