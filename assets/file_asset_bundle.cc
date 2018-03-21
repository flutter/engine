// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/assets/file_asset_bundle.h"

#include "lib/fxl/files/file.h"
#include "lib/fxl/files/path.h"

namespace blink {

FileAssetBundle::FileAssetBundle() = default;

FileAssetBundle::~FileAssetBundle() = default;

// |blink::AssetResolver|
bool FileAssetBundle::IsValid() const {
  return true;
}

// |blink::AssetResolver|
bool FileAssetBundle::GetAsBuffer(const std::string& asset_name,
                                  std::vector<uint8_t>* data) const {
  return files::ReadFileToVector(files::AbsolutePath(asset_name), data);
}

}  // namespace blink
