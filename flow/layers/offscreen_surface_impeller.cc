// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/layers/offscreen_surface_impeller.h"

#include "fml/synchronization/count_down_latch.h"
#include "impeller/display_list/dl_dispatcher.h"
#include "impeller/renderer/command_buffer.h"
#include "include/core/SkSize.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkPixmap.h"
#include "third_party/skia/include/encode/SkPngEncoder.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/gpu/ganesh/SkSurfaceGanesh.h"

namespace flutter {

OffscreenSurfaceImpeller::OffscreenSurfaceImpeller(
    const std::shared_ptr<impeller::AiksContext>& surface_context,
    const SkISize& size)
    : surface_context_(surface_context) {}

sk_sp<SkData> OffscreenSurfaceImpeller::GetRasterData(bool compressed) const {
  auto display_list = builder_->Build();
  if (!display_list) {
    return nullptr;
  }

  auto raw_size = display_list->bounds();
  SkIRect sk_cull_rect = SkIRect::MakeWH(raw_size.width(), raw_size.height());
  impeller::Rect cull_rect = impeller::Rect::MakeLTRB(
      0, 0, sk_cull_rect.width(), sk_cull_rect.height());
  impeller::DlDispatcher impeller_dispatcher(cull_rect);
  display_list->Dispatch(impeller_dispatcher, sk_cull_rect);
  auto picture = impeller_dispatcher.EndRecordingAsPicture();
  auto image = picture.ToImage(*surface_context_,
                               {sk_cull_rect.width(), sk_cull_rect.height()});
  if (!image) {
    return nullptr;
  }
  auto texture = image->GetTexture();

  if (!texture) {
    return nullptr;
  }

  auto cmd_buffer = surface_context_->GetContext()->CreateCommandBuffer();
  auto blit_pass = cmd_buffer->CreateBlitPass();
  auto buffer_size =
      texture->GetTextureDescriptor().GetByteSizeOfBaseMipLevel();

  impeller::DeviceBufferDescriptor desc;
  desc.size = buffer_size;
  desc.storage_mode = impeller::StorageMode::kHostVisible;
  auto device_buffer =
      surface_context_->GetContext()->GetResourceAllocator()->CreateBuffer(
          desc);
  if (!device_buffer) {
    return nullptr;
  }

  if (!blit_pass->AddCopy(texture, device_buffer)) {
    return nullptr;
  }
  fml::CountDownLatch latch(1u);
  bool success = false;
  auto submit_result =
      cmd_buffer->SubmitCommands([&](impeller::CommandBuffer::Status status) {
        if (status == impeller::CommandBuffer::Status::kCompleted) {
          success = true;
        }
        latch.CountDown();
      });
  if (!submit_result) {
    return nullptr;
  }
  latch.Wait();
  if (!success) {
    return nullptr;
  }

  auto buffer_view = device_buffer->AsBufferView();

  SkColorType color_type = SkColorType::kRGBA_8888_SkColorType;
  auto impeller_format = texture->GetTextureDescriptor().format;
  if (impeller_format == impeller::PixelFormat::kR8G8B8A8UNormInt) {
    color_type = SkColorType::kRGBA_8888_SkColorType;
  } else if (impeller_format == impeller::PixelFormat::kB8G8R8A8UNormInt) {
    color_type = SkColorType::kBGRA_8888_SkColorType;
  } else {
    FML_LOG(ERROR) << "Unsupported surface format.";
  }

  SkISize image_size =
      SkISize::Make(image->GetSize().width, image->GetSize().height);
  SkImageInfo image_info = SkImageInfo::Make(image_size, color_type,
                                             SkAlphaType::kPremul_SkAlphaType);

  SkBitmap bitmap;
  auto bytes_per_pixel = image_info.bytesPerPixel();
  bitmap.installPixels(image_info, buffer_view.contents,
                       image_size.width() * bytes_per_pixel);
  bitmap.setImmutable();

  sk_sp<SkImage> raster_image = SkImages::RasterFromBitmap(bitmap);

  // If the caller want the pixels to be compressed, there is a Skia utility to
  // compress to PNG. Use that.
  if (compressed) {
    return SkPngEncoder::Encode(nullptr, raster_image.get(), {});
  }

  // Copy it into a bitmap and return the same.
  SkPixmap pixmap;
  if (!raster_image->peekPixels(&pixmap)) {
    FML_LOG(ERROR) << "Screenshot: unable to obtain bitmap pixels";
    return nullptr;
  }
  return SkData::MakeWithCopy(pixmap.addr32(), pixmap.computeByteSize());
}

DlCanvas* OffscreenSurfaceImpeller::GetCanvas() {
  return builder_.get();
}

bool OffscreenSurfaceImpeller::IsValid() const {
  return surface_context_ != nullptr;
}

}  // namespace flutter
