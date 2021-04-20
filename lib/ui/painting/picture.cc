// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/picture.h"

#include <memory>

#include "flutter/fml/make_copyable.h"
#include "flutter/lib/ui/painting/canvas.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_args.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"
#include "third_party/tonic/dart_persistent_value.h"
#include "third_party/tonic/logging/dart_invoke.h"

namespace flutter {
namespace {
void PostUpdateUITask(fml::RefPtr<fml::TaskRunner> ui_task_runner,
                      std::function<void(sk_sp<SkImage>)> ui_task,
                      sk_sp<SkImage> raster_image) {
  fml::TaskRunner::RunNowOrPostTask(
      ui_task_runner, [ui_task, raster_image]() { ui_task(raster_image); });
}

void DrawSnapshotFallback(
    fml::WeakPtr<flutter::SnapshotDelegate> snapshot_delegate,
    sk_sp<SkPicture> picture,
    const SkISize& picture_bounds,
    fml::RefPtr<fml::TaskRunner> ui_task_runner,
    std::function<void(sk_sp<SkImage>)> ui_task) {
  auto raster_image =
      snapshot_delegate->DrawSnapshotAndTransferToHost(picture, picture_bounds);
  PostUpdateUITask(ui_task_runner, ui_task, raster_image);
}
}  // namespace

IMPLEMENT_WRAPPERTYPEINFO(ui, Picture);

#define FOR_EACH_BINDING(V) \
  V(Picture, toImage)       \
  V(Picture, dispose)       \
  V(Picture, GetAllocationSize)

DART_BIND_ALL(Picture, FOR_EACH_BINDING)

fml::RefPtr<Picture> Picture::Create(
    Dart_Handle dart_handle,
    flutter::SkiaGPUObject<SkPicture> picture) {
  auto canvas_picture = fml::MakeRefCounted<Picture>(std::move(picture));

  canvas_picture->AssociateWithDartWrapper(dart_handle);
  return canvas_picture;
}

Picture::Picture(flutter::SkiaGPUObject<SkPicture> picture)
    : picture_(std::move(picture)) {}

Picture::~Picture() = default;

Dart_Handle Picture::toImage(uint32_t width,
                             uint32_t height,
                             Dart_Handle raw_image_callback) {
  if (!picture_.get()) {
    return tonic::ToDart("Picture is null");
  }

  return RasterizeToImage(picture_.get(), width, height, raw_image_callback);
}

void Picture::dispose() {
  picture_.reset();
  ClearDartWrapper();
}

size_t Picture::GetAllocationSize() const {
  if (auto picture = picture_.get()) {
    return picture->approximateBytesUsed() + sizeof(Picture);
  } else {
    return sizeof(Picture);
  }
}

Dart_Handle Picture::RasterizeToImage(sk_sp<SkPicture> picture,
                                      uint32_t width,
                                      uint32_t height,
                                      Dart_Handle raw_image_callback) {
  if (Dart_IsNull(raw_image_callback) || !Dart_IsClosure(raw_image_callback)) {
    return tonic::ToDart("Image callback was invalid");
  }

  if (width == 0 || height == 0) {
    return tonic::ToDart("Image dimensions for scene were invalid.");
  }

  auto* dart_state = UIDartState::Current();
  auto image_callback = std::make_unique<tonic::DartPersistentValue>(
      dart_state, raw_image_callback);
  auto unref_queue = dart_state->GetSkiaUnrefQueue();
  auto ui_task_runner = dart_state->GetTaskRunners().GetUITaskRunner();
  auto raster_task_runner = dart_state->GetTaskRunners().GetRasterTaskRunner();
  auto io_task_runner = dart_state->GetTaskRunners().GetIOTaskRunner();
  fml::WeakPtr<IOManager> io_manager = dart_state->GetIOManager();
  auto snapshot_delegate = dart_state->GetSnapshotDelegate();

  // We can't create an image on this task runner because we don't have a
  // graphics context. Even if we did, it would be slow anyway. Also, this
  // thread owns the sole reference to the layer tree. So we flatten the layer
  // tree into a picture and use that as the thread transport mechanism.

  auto picture_bounds = SkISize::Make(width, height);

  auto ui_task = fml::MakeCopyable([image_callback = std::move(image_callback),
                                    unref_queue](
                                       sk_sp<SkImage> raster_image) mutable {
    auto dart_state = image_callback->dart_state().lock();
    if (!dart_state) {
      // The root isolate could have died in the meantime.
      return;
    }
    tonic::DartState::Scope scope(dart_state);

    if (!raster_image) {
      tonic::DartInvoke(image_callback->Get(), {Dart_Null()});
      return;
    }

    auto dart_image = CanvasImage::Create();
    dart_image->set_image({std::move(raster_image), std::move(unref_queue)});
    auto* raw_dart_image = tonic::ToDart(std::move(dart_image));

    // All done!
    tonic::DartInvoke(image_callback->Get(), {raw_dart_image});

    // image_callback is associated with the Dart isolate and must be deleted
    // on the UI thread.
    image_callback.reset();
  });

  // Kick things off on the raster rask runner.
  fml::TaskRunner::RunNowOrPostTask(
      raster_task_runner,
      [raster_task_runner, io_task_runner, io_manager, ui_task_runner,
       snapshot_delegate, picture, picture_bounds, ui_task] {
        auto raster_texture =
            snapshot_delegate->DrawSnapshotTexture(picture, picture_bounds);
        if (raster_texture) {
          fml::TaskRunner::RunNowOrPostTask(
              io_task_runner,
              [raster_task_runner, ui_task_runner, ui_task, io_manager,
               raster_texture, picture, picture_bounds, snapshot_delegate] {
                auto resource_context = io_manager->GetResourceContext();
                auto raster_image = SnapshotDelegate::TextureToImage(
                    resource_context.get(), raster_texture);
                if (raster_image) {
                  PostUpdateUITask(ui_task_runner, ui_task, raster_image);
                } else {
                  fml::TaskRunner::RunNowOrPostTask(
                      raster_task_runner, [ui_task_runner, snapshot_delegate,
                                           picture, picture_bounds, ui_task] {
                        DrawSnapshotFallback(snapshot_delegate, picture,
                                             picture_bounds, ui_task_runner,
                                             ui_task);
                      });
                }
              });
        } else {
          DrawSnapshotFallback(snapshot_delegate, picture, picture_bounds,
                               ui_task_runner, ui_task);
        }
      });

  return Dart_Null();
}

}  // namespace flutter
