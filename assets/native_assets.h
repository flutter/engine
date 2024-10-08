// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_ASSETS_NATIVE_ASSETS_H_
#define FLUTTER_ASSETS_NATIVE_ASSETS_H_

#include <memory>
#include <vector>

#include "flutter/assets/asset_manager.h"

namespace flutter {

class NativeAssetsManager {
 public:
  NativeAssetsManager() = default;
  ~NativeAssetsManager() = default;

  void RegisterNativeAssets(const std::shared_ptr<AssetManager>& asset_manager);

  std::vector<std::string> LookupNativeAsset(const std::string& asset_id);

  std::string AvailableNativeAssets();

 private:
  std::unordered_map<std::string, std::vector<std::string>> parsed_mapping_;

  FML_DISALLOW_COPY_AND_ASSIGN(NativeAssetsManager);
};

}  // namespace flutter

#endif  // FLUTTER_ASSETS_NATIVE_ASSETS_H_
