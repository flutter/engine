// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_ASSETS_ASSET_MANAGER_H_
#define FLUTTER_ASSETS_ASSET_MANAGER_H_

#include <deque>
#include <memory>
#include <string>

#include "flutter/assets/asset_resolver.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"

namespace flutter {

class AssetManager final : public AssetResolver {
 public:
  AssetManager();

  ~AssetManager() override;

  void PushFront(std::unique_ptr<AssetResolver> resolver);

  void PushBack(std::unique_ptr<AssetResolver> resolver);

  //--------------------------------------------------------------------------
  /// @brief      Replaces asset resolvers handled by this AssetManager that are
  ///             of the specified `type` with the resolvers provided in
  ///             `updated_asset_resolvers`. Updatable AssetResolvers
  ///             are removed and replaced with the next available resolver
  ///             in `updated_asset_resolvers`.
  ///
  ///             AssetResolvers should be updated when the exisitng resolver
  ///             becomes obsolete and a newer one becomes available that
  ///             provides updated access to the same type of assets as the
  ///             existing one. This update process is meant to be performed at
  ///             runtime.
  ///
  ///             If less resolvers are provided than existing resolvers of
  ///             matching type, the the extra existing resolvers will
  ///             be removed without replacement. If more resolvers are provided
  ///             than existing matching resolvers, then the extra provided
  ///             resolvers will be added to the end of the AssetManager
  ///             resolvers queue.
  ///
  /// @param[in]  asset_resolvers  The asset resolvers to replace updatable
  ///                              existing resolvers with.
  ///
  /// @param[in]  type  The type of AssetResolver to update. Only resolvers of
  ///                   the specified type will be replaced by an updated
  ///                   resolver.
  ///
  void UpdateResolversByType(
      std::vector<std::unique_ptr<AssetResolver>>& updated_asset_resolvers,
      AssetResolver::AssetResolverType type);

  std::deque<std::unique_ptr<AssetResolver>> TakeResolvers();

  // |AssetResolver|
  bool IsValid() const override;

  // |AssetResolver|
  bool IsValidAfterAssetManagerChange() const override;

  // |AssetResolver|
  AssetResolver::AssetResolverType GetType() const override;

  // |AssetResolver|
  std::unique_ptr<fml::Mapping> GetAsMapping(
      const std::string& asset_name) const override;

  // |AssetResolver|
  std::vector<std::unique_ptr<fml::Mapping>> GetAsMappings(
      const std::string& asset_pattern) const override;

  size_t Size() { return resolvers_.size(); }

 private:
  std::deque<std::unique_ptr<AssetResolver>> resolvers_;

  FML_DISALLOW_COPY_AND_ASSIGN(AssetManager);
};

}  // namespace flutter

#endif  // FLUTTER_ASSETS_ASSET_MANAGER_H_
