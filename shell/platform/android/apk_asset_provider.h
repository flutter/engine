// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_ASSETS_APK_ASSET_PROVIDER_H_
#define FLUTTER_ASSETS_APK_ASSET_PROVIDER_H_

#include <android/asset_manager_jni.h>
#include <jni.h>
#include <memory>

#include "flutter/assets/asset_resolver.h"
#include "flutter/fml/memory/ref_counted.h"
#include "flutter/fml/platform/android/scoped_java_ref.h"

namespace flutter {

class APKAssetProviderImpl final : public AssetResolver {
 public:
  explicit APKAssetProviderImpl(JNIEnv* env,
                                jobject assetManager,
                                std::string directory);
  ~APKAssetProviderImpl() override;

 private:
  fml::jni::ScopedJavaGlobalRef<jobject> java_asset_manager_;
  AAssetManager* assetManager_;
  const std::string directory_;

  // |flutter::AssetResolver|
  bool IsValid() const override;

  // |flutter::AssetResolver|
  bool IsValidAfterAssetManagerChange() const override;

  // |AssetResolver|
  AssetResolver::AssetResolverType GetType() const override;

  // |flutter::AssetResolver|
  std::unique_ptr<fml::Mapping> GetAsMapping(
      const std::string& asset_name) const override;
  friend class APKAssetProvider;
  FML_DISALLOW_COPY_AND_ASSIGN(APKAssetProviderImpl);
};

class APKAssetProvider final : public AssetResolver {
 public:
  explicit APKAssetProvider(JNIEnv* env,
                            jobject assetManager,
                            std::string directory)
      : impl_(std::make_shared<APKAssetProviderImpl>(env,
                                                     assetManager,
                                                     std::move(directory))) {}

  explicit APKAssetProvider(std::shared_ptr<APKAssetProviderImpl> impl)
      : impl_(impl) {}
  ~APKAssetProvider() = default;

  std::unique_ptr<APKAssetProvider> Clone() {
    return std::make_unique<APKAssetProvider>(impl_);
  }

 private:
  std::shared_ptr<APKAssetProviderImpl> impl_;

  // |flutter::AssetResolver|
  bool IsValid() const override { return impl_->IsValid(); }

  // |flutter::AssetResolver|
  bool IsValidAfterAssetManagerChange() const override {
    return impl_->IsValidAfterAssetManagerChange();
  }

  // |AssetResolver|
  AssetResolver::AssetResolverType GetType() const override {
    return impl_->GetType();
  }

  // |flutter::AssetResolver|
  std::unique_ptr<fml::Mapping> GetAsMapping(
      const std::string& asset_name) const override {
    return impl_->GetAsMapping(asset_name);
  }

  FML_DISALLOW_COPY_AND_ASSIGN(APKAssetProvider);
};

}  // namespace flutter

#endif  // FLUTTER_ASSETS_APK_ASSET_PROVIDER_H
