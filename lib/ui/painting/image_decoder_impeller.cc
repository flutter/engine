// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/image_decoder_impeller.h"

#include <memory>

#include "flutter/fml/closure.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/fml/trace_event.h"
#include "flutter/impeller/core/allocator.h"
#include "flutter/impeller/display_list/dl_image_impeller.h"
#include "flutter/impeller/renderer/command_buffer.h"
#include "flutter/impeller/renderer/context.h"
#include "impeller/base/strings.h"
#include "impeller/core/device_buffer.h"
#include "impeller/core/formats.h"
#include "impeller/display_list/skia_conversions.h"
#include "impeller/geometry/size.h"
#include "third_party/skia/include/core/SkAlphaType.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkColorType.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkMallocPixelRef.h"
#include "third_party/skia/include/core/SkPixelRef.h"
#include "third_party/skia/include/core/SkPixmap.h"
#include "third_party/skia/include/core/SkPoint.h"
#include "third_party/skia/include/core/SkSize.h"

namespace flutter {

namespace {
/**
 *  Loads the gamut as a set of three points (triangle).
 */
void LoadGamut(SkPoint abc[3], const skcms_Matrix3x3& xyz) {
  // rx = rX / (rX + rY + rZ)
  // ry = rY / (rX + rY + rZ)
  // gx, gy, bx, and gy are calculated similarly.
  for (int index = 0; index < 3; index++) {
    float sum = xyz.vals[index][0] + xyz.vals[index][1] + xyz.vals[index][2];
    abc[index].fX = xyz.vals[index][0] / sum;
    abc[index].fY = xyz.vals[index][1] / sum;
  }
}

/**
 *  Calculates the area of the triangular gamut.
 */
float CalculateArea(SkPoint abc[3]) {
  const SkPoint& a = abc[0];
  const SkPoint& b = abc[1];
  const SkPoint& c = abc[2];
  return 0.5f * fabsf(a.fX * b.fY + b.fX * c.fY - a.fX * c.fY - c.fX * b.fY -
                      b.fX * a.fY);
}

// Note: This was calculated from SkColorSpace::MakeSRGB().
static constexpr float kSrgbGamutArea = 0.0982f;

// Source:
// https://source.chromium.org/chromium/_/skia/skia.git/+/393fb1ec80f41d8ad7d104921b6920e69749fda1:src/codec/SkAndroidCodec.cpp;l=67;drc=46572b4d445f41943059d0e377afc6d6748cd5ca;bpv=1;bpt=0
bool IsWideGamut(const SkColorSpace* color_space) {
  if (!color_space) {
    return false;
  }
  skcms_Matrix3x3 xyzd50;
  color_space->toXYZD50(&xyzd50);
  SkPoint rgb[3];
  LoadGamut(rgb, xyzd50);
  float area = CalculateArea(rgb);
  return area > kSrgbGamutArea;
}
}  // namespace

ImageDecoderImpeller::ImageDecoderImpeller(
    const TaskRunners& runners,
    std::shared_ptr<fml::ConcurrentTaskRunner> concurrent_task_runner,
    const fml::WeakPtr<IOManager>& io_manager,
    bool supports_wide_gamut,
    const std::shared_ptr<fml::SyncSwitch>& gpu_disabled_switch)
    : ImageDecoder(runners, std::move(concurrent_task_runner), io_manager),
      supports_wide_gamut_(supports_wide_gamut),
      gpu_disabled_switch_(gpu_disabled_switch) {
  std::promise<std::shared_ptr<impeller::Context>> context_promise;
  context_ = context_promise.get_future();
  runners_.GetIOTaskRunner()->PostTask(fml::MakeCopyable(
      [promise = std::move(context_promise), io_manager]() mutable {
        promise.set_value(io_manager ? io_manager->GetImpellerContext()
                                     : nullptr);
      }));
}

ImageDecoderImpeller::~ImageDecoderImpeller() = default;

static SkColorType ChooseCompatibleColorType(SkColorType type) {
  switch (type) {
    case kRGBA_F32_SkColorType:
      return kRGBA_F16_SkColorType;
    default:
      return kRGBA_8888_SkColorType;
  }
}

static SkAlphaType ChooseCompatibleAlphaType(SkAlphaType type) {
  return type;
}

DecompressResult ImageDecoderImpeller::DecompressTexture(
    ImageDescriptor* descriptor,
    SkISize target_size,
    impeller::ISize max_texture_size,
    bool supports_wide_gamut,
    const std::shared_ptr<impeller::Allocator>& allocator) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!descriptor) {
    std::string decode_error("Invalid descriptor (should never happen)");
    FML_DLOG(ERROR) << decode_error;
    return DecompressResult{.decode_error = decode_error};
  }

  target_size.set(std::min(static_cast<int32_t>(max_texture_size.width),
                           target_size.width()),
                  std::min(static_cast<int32_t>(max_texture_size.height),
                           target_size.height()));

  const SkISize source_size = descriptor->image_info().dimensions();
  auto decode_size = source_size;
  if (descriptor->is_compressed()) {
    decode_size = descriptor->get_scaled_dimensions(std::max(
        static_cast<float>(target_size.width()) / source_size.width(),
        static_cast<float>(target_size.height()) / source_size.height()));
  }

  //----------------------------------------------------------------------------
  /// 1. Decode the image.
  ///

  const auto base_image_info = descriptor->image_info();
  const bool is_wide_gamut =
      supports_wide_gamut ? IsWideGamut(base_image_info.colorSpace()) : false;
  SkAlphaType alpha_type =
      ChooseCompatibleAlphaType(base_image_info.alphaType());
  SkImageInfo image_info;
  if (is_wide_gamut) {
    SkColorType color_type = alpha_type == SkAlphaType::kOpaque_SkAlphaType
                                 ? kBGR_101010x_XR_SkColorType
                                 : kRGBA_F16_SkColorType;
    image_info =
        base_image_info.makeWH(decode_size.width(), decode_size.height())
            .makeColorType(color_type)
            .makeAlphaType(alpha_type)
            .makeColorSpace(SkColorSpace::MakeSRGB());
  } else {
    image_info =
        base_image_info.makeWH(decode_size.width(), decode_size.height())
            .makeColorType(
                ChooseCompatibleColorType(base_image_info.colorType()))
            .makeAlphaType(alpha_type);
  }

  const auto pixel_format =
      impeller::skia_conversions::ToPixelFormat(image_info.colorType());
  if (!pixel_format.has_value()) {
    std::string decode_error(impeller::SPrintF(
        "Codec pixel format is not supported (SkColorType=%d)",
        image_info.colorType()));
    FML_DLOG(ERROR) << decode_error;
    return DecompressResult{.decode_error = decode_error};
  }

  auto bitmap = std::make_shared<SkBitmap>();
  bitmap->setInfo(image_info);
  auto bitmap_allocator = std::make_shared<ImpellerAllocator>(allocator);

  if (descriptor->is_compressed()) {
    if (!bitmap->tryAllocPixels(bitmap_allocator.get())) {
      std::string decode_error(
          "Could not allocate intermediate for image decompression.");
      FML_DLOG(ERROR) << decode_error;
      return DecompressResult{.decode_error = decode_error};
    }
    // Decode the image into the image generator's closest supported size.
    if (!descriptor->get_pixels(bitmap->pixmap())) {
      std::string decode_error("Could not decompress image.");
      FML_DLOG(ERROR) << decode_error;
      return DecompressResult{.decode_error = decode_error};
    }
  } else {
    auto temp_bitmap = std::make_shared<SkBitmap>();
    temp_bitmap->setInfo(base_image_info);
    auto pixel_ref = SkMallocPixelRef::MakeWithData(
        base_image_info, descriptor->row_bytes(), descriptor->data());
    temp_bitmap->setPixelRef(pixel_ref, 0, 0);

    if (!bitmap->tryAllocPixels(bitmap_allocator.get())) {
      std::string decode_error(
          "Could not allocate intermediate for pixel conversion.");
      FML_DLOG(ERROR) << decode_error;
      return DecompressResult{.decode_error = decode_error};
    }
    temp_bitmap->readPixels(bitmap->pixmap());
    bitmap->setImmutable();
  }

  // If the image is unpremultiplied, fix it.
  if (alpha_type == SkAlphaType::kUnpremul_SkAlphaType) {
    // Single copy of ImpellerAllocator crashes.
    auto premul_allocator = std::make_shared<ImpellerAllocator>(allocator);
    auto premul_bitmap = std::make_shared<SkBitmap>();
    premul_bitmap->setInfo(bitmap->info().makeAlphaType(kPremul_SkAlphaType));
    if (!premul_bitmap->tryAllocPixels(premul_allocator.get())) {
      std::string decode_error(
          "Could not allocate intermediate for premultiplication conversion.");
      FML_DLOG(ERROR) << decode_error;
      return DecompressResult{.decode_error = decode_error};
    }
    // readPixels() handles converting pixels to premultiplied form.
    bitmap->readPixels(premul_bitmap->pixmap());
    premul_bitmap->setImmutable();
    bitmap_allocator = premul_allocator;
    bitmap = premul_bitmap;
  }

  std::shared_ptr<impeller::DeviceBuffer> buffer =
      bitmap_allocator->GetDeviceBuffer();
  if (!buffer) {
    return DecompressResult{.decode_error = "Unable to get device buffer"};
  }
  buffer->Flush();

  std::optional<SkImageInfo> resize_info =
      bitmap->dimensions() == target_size
          ? std::nullopt
          : std::optional<SkImageInfo>(image_info.makeDimensions(target_size));
  return DecompressResult{.device_buffer = std::move(buffer),
                          .sk_bitmap = bitmap,
                          .image_info = bitmap->info(),
                          .resize_info = resize_info};
}

// static
std::pair<sk_sp<DlImage>, std::string>
ImageDecoderImpeller::UnsafeUploadTextureToPrivate(
    const std::shared_ptr<impeller::Context>& context,
    const std::shared_ptr<impeller::DeviceBuffer>& buffer,
    const SkImageInfo& image_info,
    const std::optional<SkImageInfo>& resize_info,
    bool create_mips) {
  const auto pixel_format =
      impeller::skia_conversions::ToPixelFormat(image_info.colorType());
  // mips should still be created if the source image is being resized.
  const bool should_create_mips = create_mips || resize_info.has_value();
  if (!pixel_format) {
    std::string decode_error(impeller::SPrintF(
        "Unsupported pixel format (SkColorType=%d)", image_info.colorType()));
    FML_DLOG(ERROR) << decode_error;
    return std::make_pair(nullptr, decode_error);
  }

  impeller::TextureDescriptor texture_descriptor;
  texture_descriptor.storage_mode = impeller::StorageMode::kDevicePrivate;
  texture_descriptor.format = pixel_format.value();
  texture_descriptor.size = {image_info.width(), image_info.height()};
  texture_descriptor.mip_count =
      should_create_mips ? texture_descriptor.size.MipCount() : 1;
  texture_descriptor.compression_type = impeller::CompressionType::kLossy;

  auto dest_texture =
      context->GetResourceAllocator()->CreateTexture(texture_descriptor);
  if (!dest_texture) {
    std::string decode_error("Could not create Impeller texture.");
    FML_DLOG(ERROR) << decode_error;
    return std::make_pair(nullptr, decode_error);
  }

  dest_texture->SetLabel(
      impeller::SPrintF("ui.Image(%p)", dest_texture.get()).c_str());

  auto command_buffer = context->CreateCommandBuffer();
  if (!command_buffer) {
    std::string decode_error(
        "Could not create command buffer for mipmap generation.");
    FML_DLOG(ERROR) << decode_error;
    return std::make_pair(nullptr, decode_error);
  }
  command_buffer->SetLabel("Mipmap Command Buffer");

  auto blit_pass = command_buffer->CreateBlitPass();
  if (!blit_pass) {
    std::string decode_error(
        "Could not create blit pass for mipmap generation.");
    FML_DLOG(ERROR) << decode_error;
    return std::make_pair(nullptr, decode_error);
  }
  blit_pass->SetLabel("Mipmap Blit Pass");
  blit_pass->AddCopy(impeller::DeviceBuffer::AsBufferView(buffer),
                     dest_texture);
  if (texture_descriptor.mip_count > 1) {
    blit_pass->GenerateMipmap(dest_texture);
  }

  std::shared_ptr<impeller::Texture> result_texture = dest_texture;
  if (resize_info.has_value()) {
    impeller::TextureDescriptor resize_desc;
    resize_desc.storage_mode = impeller::StorageMode::kDevicePrivate;
    resize_desc.format = pixel_format.value();
    resize_desc.size = {resize_info->width(), resize_info->height()};
    resize_desc.mip_count = create_mips ? resize_desc.size.MipCount() : 1;
    resize_desc.compression_type = impeller::CompressionType::kLossy;

    auto resize_texture =
        context->GetResourceAllocator()->CreateTexture(resize_desc);
    if (!resize_texture) {
      std::string decode_error("Could not create resized Impeller texture.");
      FML_DLOG(ERROR) << decode_error;
      return std::make_pair(nullptr, decode_error);
    }

    blit_pass->AddCopy(/*source=*/dest_texture, /*destination=*/resize_texture);
    if (resize_desc.mip_count > 1) {
      blit_pass->GenerateMipmap(resize_texture);
    }

    result_texture = std::move(resize_texture);
  }
  blit_pass->EncodeCommands(context->GetResourceAllocator());

  if (!context->GetCommandQueue()->Submit({command_buffer}).ok()) {
    std::string decode_error("Failed to submit image deocding command buffer.");
    FML_DLOG(ERROR) << decode_error;
    return std::make_pair(nullptr, decode_error);
  }

  return std::make_pair(
      impeller::DlImageImpeller::Make(std::move(result_texture)),
      std::string());
}

void ImageDecoderImpeller::UploadTextureToPrivate(
    ImageResult result,
    const std::shared_ptr<impeller::Context>& context,
    const std::shared_ptr<impeller::DeviceBuffer>& buffer,
    const SkImageInfo& image_info,
    const std::shared_ptr<SkBitmap>& bitmap,
    const std::optional<SkImageInfo>& resize_info,
    const std::shared_ptr<fml::SyncSwitch>& gpu_disabled_switch,
    bool create_mips) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!context) {
    result(nullptr, "No Impeller context is available");
    return;
  }
  if (!buffer) {
    result(nullptr, "No Impeller device buffer is available");
    return;
  }

  gpu_disabled_switch->Execute(
      fml::SyncSwitch::Handlers()
          .SetIfFalse(
              [&result, context, buffer, image_info, resize_info, create_mips] {
                sk_sp<DlImage> image;
                std::string decode_error;
                std::tie(image, decode_error) = std::tie(image, decode_error) =
                    UnsafeUploadTextureToPrivate(context, buffer, image_info,
                                                 resize_info, create_mips);
                result(image, decode_error);
              })
          .SetIfTrue([&result, context, buffer, image_info, resize_info,
                      create_mips] {
            // The `result` function must be copied in the capture list for each
            // closure or the stack allocated callback will be cleared by the
            // time to closure is executed later.
            context->StoreTaskForGPU(
                [result, context, buffer, image_info, resize_info,
                 create_mips]() {
                  sk_sp<DlImage> image;
                  std::string decode_error;
                  std::tie(image, decode_error) = std::tie(image,
                                                           decode_error) =
                      UnsafeUploadTextureToPrivate(context, buffer, image_info,
                                                   resize_info, create_mips);
                  result(image, decode_error);
                },
                [result]() {
                  result(nullptr,
                         "Image upload failed due to loss of GPU access.");
                });
          }));
}

std::pair<sk_sp<DlImage>, std::string>
ImageDecoderImpeller::UploadTextureToStorage(
    const std::shared_ptr<impeller::Context>& context,
    std::shared_ptr<SkBitmap> bitmap) {
  TRACE_EVENT0("impeller", __FUNCTION__);
  if (!context) {
    return std::make_pair(nullptr, "No Impeller context is available");
  }
  if (!bitmap) {
    return std::make_pair(nullptr, "No texture bitmap is available");
  }
  const auto image_info = bitmap->info();
  const auto pixel_format =
      impeller::skia_conversions::ToPixelFormat(image_info.colorType());
  if (!pixel_format) {
    std::string decode_error(impeller::SPrintF(
        "Unsupported pixel format (SkColorType=%d)", image_info.colorType()));
    FML_DLOG(ERROR) << decode_error;
    return std::make_pair(nullptr, decode_error);
  }

  impeller::TextureDescriptor texture_descriptor;
  texture_descriptor.storage_mode = impeller::StorageMode::kDevicePrivate;
  texture_descriptor.format = pixel_format.value();
  texture_descriptor.size = {image_info.width(), image_info.height()};
  texture_descriptor.mip_count = 1;

  auto texture =
      context->GetResourceAllocator()->CreateTexture(texture_descriptor);
  if (!texture) {
    std::string decode_error("Could not create Impeller texture.");
    FML_DLOG(ERROR) << decode_error;
    return std::make_pair(nullptr, decode_error);
  }

  auto mapping = std::make_shared<fml::NonOwnedMapping>(
      reinterpret_cast<const uint8_t*>(bitmap->getAddr(0, 0)),  // data
      texture_descriptor.GetByteSizeOfBaseMipLevel(),           // size
      [bitmap](auto, auto) mutable { bitmap.reset(); }          // proc
  );

  if (!texture->SetContents(mapping)) {
    std::string decode_error("Could not copy contents into Impeller texture.");
    FML_DLOG(ERROR) << decode_error;
    return std::make_pair(nullptr, decode_error);
  }

  texture->SetLabel(impeller::SPrintF("ui.Image(%p)", texture.get()).c_str());
  return std::make_pair(impeller::DlImageImpeller::Make(std::move(texture)),
                        std::string());
}

// |ImageDecoder|
void ImageDecoderImpeller::Decode(fml::RefPtr<ImageDescriptor> descriptor,
                                  uint32_t target_width,
                                  uint32_t target_height,
                                  const ImageResult& p_result) {
  FML_DCHECK(descriptor);
  FML_DCHECK(p_result);

  // Wrap the result callback so that it can be invoked from any thread.
  auto raw_descriptor = descriptor.get();
  raw_descriptor->AddRef();
  ImageResult result = [p_result,                               //
                        raw_descriptor,                         //
                        ui_runner = runners_.GetUITaskRunner()  //
  ](auto image, auto decode_error) {
    ui_runner->PostTask([raw_descriptor, p_result, image, decode_error]() {
      raw_descriptor->Release();
      p_result(std::move(image), decode_error);
    });
  };

  concurrent_task_runner_->PostTask(
      [raw_descriptor,                                            //
       context = context_.get(),                                  //
       target_size = SkISize::Make(target_width, target_height),  //
       io_runner = runners_.GetIOTaskRunner(),                    //
       result,
       supports_wide_gamut = supports_wide_gamut_,  //
       gpu_disabled_switch = gpu_disabled_switch_]() {
        if (!context) {
          result(nullptr, "No Impeller context is available");
          return;
        }
        auto max_size_supported =
            context->GetResourceAllocator()->GetMaxTextureSizeSupported();

        // Always decompress on the concurrent runner.
        auto bitmap_result = DecompressTexture(
            raw_descriptor, target_size, max_size_supported,
            supports_wide_gamut, context->GetResourceAllocator());
        if (!bitmap_result.device_buffer) {
          result(nullptr, bitmap_result.decode_error);
          return;
        }

        auto upload_texture_and_invoke_result = [result, context, bitmap_result,
                                                 gpu_disabled_switch]() {
          UploadTextureToPrivate(result, context,              //
                                 bitmap_result.device_buffer,  //
                                 bitmap_result.image_info,     //
                                 bitmap_result.sk_bitmap,      //
                                 bitmap_result.resize_info,    //
                                 gpu_disabled_switch           //
          );
        };
        // The I/O image uploads are not threadsafe on GLES.
        if (context->GetBackendType() ==
            impeller::Context::BackendType::kOpenGLES) {
          io_runner->PostTask(upload_texture_and_invoke_result);
        } else {
          upload_texture_and_invoke_result();
        }
      });
}

ImpellerAllocator::ImpellerAllocator(
    std::shared_ptr<impeller::Allocator> allocator)
    : allocator_(std::move(allocator)) {}

std::shared_ptr<impeller::DeviceBuffer> ImpellerAllocator::GetDeviceBuffer()
    const {
  return buffer_;
}

bool ImpellerAllocator::allocPixelRef(SkBitmap* bitmap) {
  if (!bitmap) {
    return false;
  }
  const SkImageInfo& info = bitmap->info();
  if (kUnknown_SkColorType == info.colorType() || info.width() < 0 ||
      info.height() < 0 || !info.validRowBytes(bitmap->rowBytes())) {
    return false;
  }

  impeller::DeviceBufferDescriptor descriptor;
  descriptor.storage_mode = impeller::StorageMode::kHostVisible;
  descriptor.size = ((bitmap->height() - 1) * bitmap->rowBytes()) +
                    (bitmap->width() * bitmap->bytesPerPixel());

  std::shared_ptr<impeller::DeviceBuffer> device_buffer =
      allocator_->CreateBuffer(descriptor);
  if (!device_buffer) {
    return false;
  }

  struct ImpellerPixelRef final : public SkPixelRef {
    ImpellerPixelRef(int w, int h, void* s, size_t r)
        : SkPixelRef(w, h, s, r) {}

    ~ImpellerPixelRef() override {}
  };

  auto pixel_ref = sk_sp<SkPixelRef>(
      new ImpellerPixelRef(info.width(), info.height(),
                           device_buffer->OnGetContents(), bitmap->rowBytes()));

  bitmap->setPixelRef(std::move(pixel_ref), 0, 0);
  buffer_ = std::move(device_buffer);
  return true;
}

}  // namespace flutter
