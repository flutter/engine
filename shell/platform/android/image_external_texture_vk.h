// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_ANDROID_IMAGE_EXTERNAL_TEXTURE_VK_H_
#define FLUTTER_SHELL_PLATFORM_ANDROID_IMAGE_EXTERNAL_TEXTURE_VK_H_

#include <cstdint>
#include <unordered_map>
#include <utility>
#include "flutter/shell/platform/android/image_external_texture.h"

#include "flutter/impeller/renderer/backend/vulkan/android_hardware_buffer_texture_source_vk.h"
#include "flutter/impeller/renderer/backend/vulkan/context_vk.h"
#include "flutter/impeller/renderer/backend/vulkan/vk.h"
#include "flutter/shell/platform/android/android_context_vulkan_impeller.h"

namespace flutter {

static constexpr size_t kImageReaderSwapchainSize = 3u;

class ImageExternalTextureVK : public ImageExternalTexture {
 public:
  ImageExternalTextureVK(
      const std::shared_ptr<impeller::ContextVK>& impeller_context,
      int64_t id,
      const fml::jni::ScopedJavaGlobalRef<jobject>&
          hardware_buffer_texture_entry,
      const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade);

  ~ImageExternalTextureVK() override;

 private:
  void Attach(PaintContext& context) override;
  void ProcessFrame(PaintContext& context, const SkRect& bounds) override;
  void Detach() override;

  sk_sp<flutter::DlImage> FindImage(uint64_t key);

  void UpdateKey(uint64_t key);

  void AddImage(sk_sp<flutter::DlImage> image, uint64_t key);

  const std::shared_ptr<impeller::ContextVK> impeller_context_;

  fml::jni::ScopedJavaGlobalRef<jobject> android_image_;

  std::array<std::pair<uint64_t, sk_sp<flutter::DlImage>>,
             kImageReaderSwapchainSize>
      images_ = {std::make_pair(0, nullptr), std::make_pair(0, nullptr),
                 std::make_pair(0, nullptr)};
  std::array<uint64_t, kImageReaderSwapchainSize> keys_ = {0, 0, 0};
};

}  // namespace flutter

#endif  // FLUTTER_SHELL_PLATFORM_ANDROID_IMAGE_EXTERNAL_TEXTURE_VK_H_
