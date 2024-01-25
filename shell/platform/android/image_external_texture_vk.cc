
#include "flutter/shell/platform/android/image_external_texture_vk.h"
#include <cstdint>

#include "flutter/impeller/core/formats.h"
#include "flutter/impeller/core/texture_descriptor.h"
#include "flutter/impeller/display_list/dl_image_impeller.h"
#include "flutter/impeller/renderer/backend/vulkan/android_hardware_buffer_texture_source_vk.h"
#include "flutter/impeller/renderer/backend/vulkan/command_buffer_vk.h"
#include "flutter/impeller/renderer/backend/vulkan/command_encoder_vk.h"
#include "flutter/impeller/renderer/backend/vulkan/texture_vk.h"
#include "flutter/shell/platform/android/ndk_helpers.h"

namespace flutter {

ImageExternalTextureVK::ImageExternalTextureVK(
    const std::shared_ptr<impeller::ContextVK>& impeller_context,
    int64_t id,
    const fml::jni::ScopedJavaGlobalRef<jobject>& image_texture_entry,
    const std::shared_ptr<PlatformViewAndroidJNI>& jni_facade)
    : ImageExternalTexture(id, image_texture_entry, jni_facade),
      impeller_context_(impeller_context) {}

ImageExternalTextureVK::~ImageExternalTextureVK() {}

void ImageExternalTextureVK::Attach(PaintContext& context) {
  if (state_ == AttachmentState::kUninitialized) {
    // First processed frame we are attached.
    state_ = AttachmentState::kAttached;
  }
}

void ImageExternalTextureVK::Detach() {}

sk_sp<flutter::DlImage> ImageExternalTextureVK::FindImage(uint64_t key) {
  for (auto i = 0u; i < kImageReaderSwapchainSize; i++) {
    if (images_[i].first == key) {
      UpdateKey(key);
      return images_[i].second;
    }
  }
  return nullptr;
}

void ImageExternalTextureVK::UpdateKey(uint64_t key) {
  if (keys_[0] == key) {
    return;
  }
  auto i = 1u;
  for (; i < kImageReaderSwapchainSize; i++) {
    if (keys_[i] == key) {
      break;
    }
  }
  for (auto j = i; j > 0; j--) {
    keys_[j] = keys_[j - 1];
  }
  keys_[0] = key;
}

void ImageExternalTextureVK::AddImage(sk_sp<flutter::DlImage> image,
                                      uint64_t key) {
  uint64_t lru_key = keys_[2];
  bool updated_image = false;
  for (auto i = 0u; i < kImageReaderSwapchainSize; i++) {
    if (images_[i].first == lru_key) {
      updated_image = true;
      images_[i] = std::make_pair(key, image);
      break;
    }
  }
  if (!updated_image) {
    keys_[0] = key;
    images_[0] = std::make_pair(key, image);
  }
  UpdateKey(key);
}

void ImageExternalTextureVK::ProcessFrame(PaintContext& context,
                                          const SkRect& bounds) {
  JavaLocalRef image = AcquireLatestImage();
  if (image.is_null()) {
    return;
  }
  JavaLocalRef old_android_image(android_image_);
  android_image_.Reset(image);
  JavaLocalRef hardware_buffer = HardwareBufferFor(android_image_);
  AHardwareBuffer* latest_hardware_buffer = AHardwareBufferFor(hardware_buffer);

  AHardwareBuffer_Desc hb_desc = {};
  flutter::NDKHelpers::AHardwareBuffer_describe(latest_hardware_buffer,
                                                &hb_desc);
  uint64_t key;
  flutter::NDKHelpers::AHardwareBuffer_getId(latest_hardware_buffer, &key);

  auto existing_image = FindImage(key);
  if (existing_image != nullptr) {
    dl_image_ = existing_image;
    return;
  }

  impeller::TextureDescriptor desc;
  desc.storage_mode = impeller::StorageMode::kDevicePrivate;
  desc.size = {static_cast<int>(bounds.width()),
               static_cast<int>(bounds.height())};
  // TODO(johnmccutchan): Use hb_desc to compute the correct format at runtime.
  desc.format = impeller::PixelFormat::kR8G8B8A8UNormInt;
  desc.mip_count = 1;

  auto texture_source =
      std::make_shared<impeller::AndroidHardwareBufferTextureSourceVK>(
          desc, impeller_context_->GetDevice(), latest_hardware_buffer,
          hb_desc);

  auto texture =
      std::make_shared<impeller::TextureVK>(impeller_context_, texture_source);
  // Transition the layout to shader read.
  {
    auto buffer = impeller_context_->CreateCommandBuffer();
    impeller::CommandBufferVK& buffer_vk =
        impeller::CommandBufferVK::Cast(*buffer);

    impeller::BarrierVK barrier;
    barrier.cmd_buffer = buffer_vk.GetEncoder()->GetCommandBuffer();
    barrier.src_access = impeller::vk::AccessFlagBits::eColorAttachmentWrite |
                         impeller::vk::AccessFlagBits::eTransferWrite;
    barrier.src_stage =
        impeller::vk::PipelineStageFlagBits::eColorAttachmentOutput |
        impeller::vk::PipelineStageFlagBits::eTransfer;
    barrier.dst_access = impeller::vk::AccessFlagBits::eShaderRead;
    barrier.dst_stage = impeller::vk::PipelineStageFlagBits::eFragmentShader;

    barrier.new_layout = impeller::vk::ImageLayout::eShaderReadOnlyOptimal;

    if (!texture->SetLayout(barrier)) {
      return;
    }
    if (!buffer->SubmitCommands()) {
      return;
    }
  }

  dl_image_ = impeller::DlImageImpeller::Make(texture);
  AddImage(dl_image_, key);
  CloseHardwareBuffer(hardware_buffer);
  // IMPORTANT: We only close the old image after texture stops referencing
  // it.
  CloseImage(old_android_image);
}

}  // namespace flutter
