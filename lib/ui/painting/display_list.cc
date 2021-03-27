// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/display_list.h"

// #include <memory>

// #include "flutter/fml/make_copyable.h"
// #include "flutter/lib/ui/painting/canvas.h"
// #include "flutter/lib/ui/ui_dart_state.h"
// #include "third_party/skia/include/core/SkBlurTypes.h"
// #include "third_party/skia/include/core/SkImage.h"
// #include "third_party/tonic/converter/dart_converter.h"
// #include "third_party/tonic/dart_args.h"
// #include "third_party/tonic/dart_library_natives.h"
// #include "third_party/tonic/dart_persistent_value.h"
// #include "third_party/tonic/logging/dart_invoke.h"

#include "flutter/flow/display_list_interpreter.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "flutter/lib/ui/painting/canvas.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/tonic/converter/dart_converter.h"
#include "third_party/tonic/dart_binding_macros.h"
#include "third_party/tonic/dart_library_natives.h"

namespace flutter {

static void DisplayList_constructor(Dart_NativeArguments args) {
  UIDartState::ThrowIfUIOperationsProhibited();
  DartCallConstructor(&DisplayList::Create, args);
}

IMPLEMENT_WRAPPERTYPEINFO(ui, DisplayList);

#define FOR_EACH_BINDING(V)             \
  V(DisplayList, toImage)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void DisplayList::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register(
      {{"DisplayList_constructor", DisplayList_constructor, 5, true},
       FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

fml::RefPtr<DisplayList> DisplayList::Create(tonic::Uint8List& ops, int numOps,
                                             tonic::DartByteData& data, int dataBytes) {
  if (numOps < 0 ||
      numOps > ops.num_elements() ||
      dataBytes < 0 || (dataBytes % sizeof(float)) != 0 ||
      dataBytes > (int) data.length_in_bytes()) {
    Dart_ThrowException(
        tonic::ToDart("DisplayList constructor called with bad list lengths."));
    return nullptr;
  }
  const uint8_t* ops_ptr = ops.data();
  std::shared_ptr<std::vector<uint8_t>> ops_vector =
    std::make_shared<std::vector<uint8_t>>(ops_ptr, ops_ptr + numOps);

  const float* data_ptr = static_cast<const float*>(data.data());
  std::shared_ptr<std::vector<float>> data_vector =
    std::make_shared<std::vector<float>>(data_ptr, data_ptr + (dataBytes / sizeof(float)));

  return fml::MakeRefCounted<DisplayList>(ops_vector, data_vector);
}

DisplayList::DisplayList(std::shared_ptr<std::vector<uint8_t>> ops_vector,
                         std::shared_ptr<std::vector<float>> data_vector)
    : ops_vector_(ops_vector),
      data_vector_(data_vector) {}

Dart_Handle DisplayList::toImage(uint32_t width,
                                 uint32_t height,
                                 Dart_Handle raw_image_callback) {
  return RasterizeToImage(ops_vector_, data_vector_, width, height, raw_image_callback);
}

Dart_Handle DisplayList::RasterizeToImage(std::shared_ptr<std::vector<uint8_t>> ops,
                                          std::shared_ptr<std::vector<float>> data,
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
      [ui_task_runner, snapshot_delegate, ops, data, picture_bounds, ui_task] {
        sk_sp<SkImage> raster_image =
            snapshot_delegate->MakeRasterSnapshot([ops = std::move(ops), data = std::move(data)](SkCanvas* canvas) {
              DisplayListInterpreter interpreter(*ops, *data);
              interpreter.Rasterize(canvas);
            }, picture_bounds);

        fml::TaskRunner::RunNowOrPostTask(
            ui_task_runner,
            [ui_task, raster_image]() { ui_task(raster_image); });
      });

  return Dart_Null();
}

// void DisplayList::dispose() {
//   picture_.reset();
//   ClearDartWrapper();ToSkRoundRect
// }

// size_t DisplayList::GetAllocationSize() const {
//   if (auto picture = picture_.get()) {
//     return picture->approximateBytesUsed() + sizeof(Picture);
//   } else {
//     return sizeof(Picture);
//   }
// }

}  // namespace flutter
