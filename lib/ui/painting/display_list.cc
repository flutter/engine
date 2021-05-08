// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/display_list.h"

#include "flutter/flow/display_list_interpreter.h"
#include "flutter/fml/logging.h"
#include "flutter/fml/make_copyable.h"
#include "flutter/lib/ui/painting/canvas.h"
#include "flutter/lib/ui/painting/image_filter.h"
#include "flutter/lib/ui/painting/path.h"
#include "flutter/lib/ui/painting/shader.h"
#include "flutter/lib/ui/ui_dart_state.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
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

#define FOR_EACH_BINDING(V) V(DisplayList, toImage)

FOR_EACH_BINDING(DART_NATIVE_CALLBACK)

void DisplayList::RegisterNatives(tonic::DartLibraryNatives* natives) {
  natives->Register(
      {{"DisplayList_constructor", DisplayList_constructor, 6, true},
       FOR_EACH_BINDING(DART_REGISTER_NATIVE)});
}

fml::RefPtr<DisplayList> DisplayList::Create(tonic::Uint8List& ops,
                                             int numOps,
                                             tonic::DartByteData& data,
                                             int numData,
                                             Dart_Handle objList) {
  if (numOps < 0 || numOps > ops.num_elements() || numData < 0 ||
      (data.length_in_bytes() % sizeof(float)) != 0 ||
      numData > (int)(data.length_in_bytes() / sizeof(float))) {
    Dart_ThrowException(
        tonic::ToDart("DisplayList constructor called with bad list lengths."));
    return nullptr;
  }
  if (Dart_IsNull(objList) || !Dart_IsList(objList)) {
    Dart_ThrowException(
        tonic::ToDart("DisplayList constructor called with bad object array."));
    return nullptr;
  }

  const uint8_t* ops_ptr = ops.data();
  std::shared_ptr<std::vector<uint8_t>> ops_vector =
      std::make_shared<std::vector<uint8_t>>(ops_ptr, ops_ptr + numOps);

  const float* data_ptr = static_cast<const float*>(data.data());
  std::shared_ptr<std::vector<float>> data_vector =
      std::make_shared<std::vector<float>>(data_ptr, data_ptr + numData);

  intptr_t numObjects = 0;
  Dart_ListLength(objList, &numObjects);
  Dart_Handle objects[numObjects];
  if (Dart_IsError(Dart_ListGetRange(objList, 0, numObjects, objects))) {
    return nullptr;
  }

  std::shared_ptr<std::vector<DisplayListRefHolder>> ref_vector =
      std::make_shared<std::vector<DisplayListRefHolder>>();
  int obj_index = 0;
  SkSamplingOptions sampling = DisplayListInterpreter::NearestSampling;
  for (uint8_t op : *ops_vector) {
    switch (op) {
      // All of the following op types have no arguments so they can
      // use continue instead of break for efficiency.
      case CanvasOp::cops_setFilterQualityNearest:
        sampling = DisplayListInterpreter::NearestSampling;
        continue;
      case CanvasOp::cops_setFilterQualityLinear:
        sampling = DisplayListInterpreter::LinearSampling;
        continue;
      case CanvasOp::cops_setFilterQualityMipmap:
        sampling = DisplayListInterpreter::MipmapSampling;
        continue;
      case CanvasOp::cops_setFilterQualityCubic:
        sampling = DisplayListInterpreter::CubicSampling;
        continue;
    }
    for (uint32_t args = DisplayListInterpreter::opArguments[op]; args != 0;
         args >>= CANVAS_OP_ARG_SHIFT) {
      switch (static_cast<CanvasOpArg>(args & CANVAS_OP_ARG_MASK)) {
        case color_filter: {
          DisplayListRefHolder holder;
          holder.colorFilter =
              tonic::DartConverter<ColorFilter*>::FromDart(objects[obj_index++])
                  ->filter();
          ref_vector->emplace_back(holder);
          break;
        }
        case image_filter: {
          DisplayListRefHolder holder;
          holder.imageFilter =
              tonic::DartConverter<ImageFilter*>::FromDart(objects[obj_index++])
                  ->filter();
          ref_vector->emplace_back(holder);
          break;
        }
        case display_list: {
          DisplayListRefHolder holder;
          holder.displayList =
              tonic::DartConverter<DisplayList*>::FromDart(objects[obj_index++])
                  ->data();
          ref_vector->emplace_back(holder);
          break;
        }
        case path: {
          DisplayListRefHolder holder;
          holder.pathData =
              tonic::DartConverter<CanvasPath*>::FromDart(objects[obj_index++])
                  ->path()
                  .serialize();
          ref_vector->emplace_back(holder);
          break;
        }
        case shader: {
          DisplayListRefHolder holder;
          holder.shader =
              tonic::DartConverter<Shader*>::FromDart(objects[obj_index++])
                  ->shader(sampling);
          ref_vector->emplace_back(holder);
          break;
        }
        case image: {
          DisplayListRefHolder holder;
          holder.image =
              tonic::DartConverter<CanvasImage*>::FromDart(objects[obj_index++])
                  ->image();
          ref_vector->emplace_back(holder);
          break;
        }
        case skpicture: {
          DisplayListRefHolder holder;
          holder.picture =
              tonic::DartConverter<Picture*>::FromDart(objects[obj_index++])
                  ->picture();
          ref_vector->emplace_back(holder);
          break;
        }
        case vertices: {
          DisplayListRefHolder holder;
          holder.vertices =
              tonic::DartConverter<Vertices*>::FromDart(objects[obj_index++])
                  ->vertices();
          ref_vector->emplace_back(holder);
          break;
        }
        case empty:
        case angle:
        case color:
        case blend_mode:
        case matrix_row3:
        case point:
        case rect:
        case round_rect:
        case scalar:
        case scalar_list:
        case uint32_list:
          break;
      }
    }
  }
  if (obj_index != numObjects) {
    FML_LOG(ERROR) << "Bad number of objects: " << obj_index
                   << " != " << numObjects;
    return nullptr;
  }

  return fml::MakeRefCounted<DisplayList>(
      std::move(ops_vector), std::move(data_vector), std::move(ref_vector));
}

DisplayList::DisplayList(
    std::shared_ptr<std::vector<uint8_t>> ops_vector,
    std::shared_ptr<std::vector<float>> data_vector,
    std::shared_ptr<std::vector<DisplayListRefHolder>> ref_vector)
    : ops_vector_(ops_vector),
      data_vector_(data_vector),
      ref_vector_(ref_vector) {}

DisplayList::~DisplayList() = default;

Dart_Handle DisplayList::toImage(uint32_t width,
                                 uint32_t height,
                                 Dart_Handle raw_image_callback) {
  return RasterizeToImage(ops_vector_, data_vector_, ref_vector_, width, height,
                          raw_image_callback);
}

void DisplayList::dispose() {
  ops_vector_.reset();
  data_vector_.reset();
  ClearDartWrapper();
}

size_t DisplayList::GetAllocationSize() const {
  return ops_vector_->size() + (data_vector_->size() * sizeof(float));
}

Dart_Handle DisplayList::RasterizeToImage(
    std::shared_ptr<std::vector<uint8_t>> ops,
    std::shared_ptr<std::vector<float>> data,
    std::shared_ptr<std::vector<DisplayListRefHolder>> refs,
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
      raster_task_runner, [ui_task_runner, snapshot_delegate, ops, data, refs,
                           picture_bounds, ui_task] {
        sk_sp<SkImage> raster_image = snapshot_delegate->MakeRasterSnapshot(
            [ops = std::move(ops), data = std::move(data),
             refs = std::move(refs)](SkCanvas* canvas) {
              DisplayListInterpreter interpreter(ops, data, refs);
              interpreter.Rasterize(canvas);
            },
            picture_bounds);

        fml::TaskRunner::RunNowOrPostTask(
            ui_task_runner,
            [ui_task, raster_image]() { ui_task(raster_image); });
      });

  return Dart_Null();
}

}  // namespace flutter
