// Copyright 2017 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_ASSETS_FILE_ASSET_BUNDLE_H_
#define FLUTTER_ASSETS_FILE_ASSET_BUNDLE_H_

#include "flutter/assets/asset_resolver.h"
#include "lib/fxl/macros.h"

namespace blink {

class FileAssetBundle final : public AssetResolver {
 public:
  FileAssetBundle();

  ~FileAssetBundle();

 private:
  // |blink::AssetResolver|
  bool IsValid() const override;

  // |blink::AssetResolver|
  bool GetAsBuffer(const std::string& asset_name,
                   std::vector<uint8_t>* data) const override;

  FXL_DISALLOW_COPY_AND_ASSIGN(FileAssetBundle);
};

}  // namespace blink

#endif  // FLUTTER_ASSETS_FILE_ASSET_BUNDLE_H_
