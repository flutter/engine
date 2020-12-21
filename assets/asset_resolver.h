// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_ASSETS_ASSET_RESOLVER_H_
#define FLUTTER_ASSETS_ASSET_RESOLVER_H_

#include <string>
#include <vector>

#include "flutter/fml/macros.h"
#include "flutter/fml/mapping.h"

namespace flutter {

class AssetResolver {
 public:
  AssetResolver() = default;

  virtual ~AssetResolver() = default;

  virtual bool IsValid() const = 0;

  //----------------------------------------------------------------------------
  /// @brief      Certain asset resolvers are still valid after the asset
  ///             manager is replaced before a hot reload, or after a new run
  ///             configuration is created during a hot restart. By preserving
  ///             these resolvers and re-inserting them into the new resolver or
  ///             run configuration, the tooling can avoid needing to sync all
  ///             application assets through the Dart devFS upon connecting to
  ///             the VM Service. Besides improving the startup performance of
  ///             running a Flutter application, it also reduces the occurance
  ///             of tool failures due to repeated network flakes caused by
  ///             damaged cables or hereto unknown bugs in the Dart HTTP server
  ///             implementation.
  ///
  /// @return     Returns whether this resolver is valid after the asset manager
  ///             or run configuration is updated.
  ///
  virtual bool IsValidAfterAssetManagerChange() const = 0;

  //----------------------------------------------------------------------------
  /// @brief      Some asset resolvers may be replaced by an updated version
  ///             during runtime. Resolvers marked `Updatable` are removed and
  ///             invalidated when an update is processed and is replaced by a
  ///             new resolver that provides the latest availablity state of
  ///             assets. This usually adds access to new assets or removes
  ///             access to old/invalid/deleted assets. For example, when
  ///             downloading a dynamic feature, Android provides a new java
  ///             asset manager that has access to the newly installed assets.
  ///             This new manager should replace the existing java asset
  ///             manager resolver. We call this replacement an update as the
  ///             old resolver is obsolete and the new one should assume
  ///             responsibility for providing access to android assets.
  ///
  /// @return     Returns whether this resolver can be updated.
  ///
  virtual bool IsUpdatable() const = 0;

  [[nodiscard]] virtual std::unique_ptr<fml::Mapping> GetAsMapping(
      const std::string& asset_name) const = 0;

  // Same as GetAsMapping() but returns mappings for all files who's name
  // matches |pattern|. Returns empty vector if no matching assets are found
  [[nodiscard]] virtual std::vector<std::unique_ptr<fml::Mapping>>
  GetAsMappings(const std::string& asset_pattern) const {
    return {};
  };

 private:
  FML_DISALLOW_COPY_AND_ASSIGN(AssetResolver);
};

}  // namespace flutter

#endif  // FLUTTER_ASSETS_ASSET_RESOLVER_H_
