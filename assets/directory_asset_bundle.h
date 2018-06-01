// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_ASSETS_DIRECTORY_ASSET_BUNDLE_H_
#define FLUTTER_ASSETS_DIRECTORY_ASSET_BUNDLE_H_

#include "flutter/assets/asset_resolver.h"
#include "flutter/fml/macros.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/unique_fd.h"

namespace blink {

class DirectoryAssetBundle : public AssetResolver {
 public:
  explicit DirectoryAssetBundle(fml::UniqueFD descriptor,
                                std::string path = "");

  ~DirectoryAssetBundle() override;

 private:
  const fml::UniqueFD descriptor_;
  const std::string path_;
  bool is_valid_ = false;

  // |blink::AssetResolver|
  bool IsValid() const override;

  // |blink::AssetResolver|
  bool GetAsBuffer(const std::string& asset_name,
                   std::vector<uint8_t>* data) const override;

  FML_DISALLOW_COPY_AND_ASSIGN(DirectoryAssetBundle);
};

}  // namespace blink

#endif  // FLUTTER_ASSETS_DIRECTORY_ASSET_BUNDLE_H_
