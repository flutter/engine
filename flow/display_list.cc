// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/display_list.h"
#include "flutter/fml/logging.h"

// #include <memory>

// #include "flutter/fml/make_copyable.h"
// #include "flutter/lib/ui/painting/canvas.h"
// #include "flutter/lib/ui/ui_dart_state.h"
// #include "third_party/skia/include/core/SkBlurTypes.h"
// #include "third_party/skia/include/core/SkImage.h"
// #include "third_party/tonic/converter/dart_converter.h"
// #include "third_party/tonic/dart_args.h"
// #include "third_party/tonic/dart_binding_macros.h"
// #include "third_party/tonic/dart_library_natives.h"
// #include "third_party/tonic/dart_persistent_value.h"
// #include "third_party/tonic/logging/dart_invoke.h"

#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkShader.h"

namespace flutter {

// IMPLEMENT_WRAPPERTYPEINFO(ui, DisplayList);

// #define FOR_EACH_BINDING(V) \
//   V(DisplayList, toImage)       \
//   V(DisplayList, dispose)       \
//   V(DisplayList, GetAllocationSize)

// DART_BIND_ALL(DisplayList, FOR_EACH_BINDING)

// fml::RefPtr<DisplayList> DisplayList::Create(
//     Dart_Handle dart_handle,
//     flutter::SkiaGPUObject<SkPicture> picture) {
//   auto canvas_picture = fml::MakeRefCounted<Picture>(std::move(picture));

//   canvas_picture->AssociateWithDartWrapper(dart_handle);
//   return canvas_picture;
// }

// DisplayList::DisplayList(flutter::SkiaGPUObject<SkPicture> picture)
//     : picture_(std::move(picture)) {}

// DisplayList::~DisplayList() = default;

#define CANVAS_OP_MAKE_STRING(name, count, imask) #name
#define CANVAS_OP_MAKE_COUNT(name, count, imask) count
#define CANVAS_OP_MAKE_IMASK(name, count, imask) imask

const std::vector<std::string> DisplayListRasterizer::opNames = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_STRING),
};

const std::vector<int> DisplayListRasterizer::opArgCounts = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_COUNT),
};

const std::vector<int> DisplayListRasterizer::opArgImask = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_IMASK),
};

DisplayListRasterizer::DisplayListRasterizer(std::vector<uint8_t> ops_vector, std::vector<float> data_vector)
    : ops_it_(ops_vector.begin()),
      ops_end_(ops_vector.end()),
      data_it_(data_vector.begin()),
      data_end_(data_vector.end()) {}

static const std::array<SkSamplingOptions, 4> filter_qualities = {
    SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone),
    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone),
    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
    SkSamplingOptions(SkCubicResampler{1 / 3.0f, 1 / 3.0f}),
};

void DisplayListRasterizer::Describe() {
  FML_LOG(ERROR) << "Starting ops: " << (ops_end_ - ops_it_) << ", data: " << (data_end_ - data_it_);
  while(HasOp()) {
    FML_LOG(ERROR) << DescribeNextOp();
    CanvasOp op = GetOp();
    for (int i = 0; i < opArgCounts[op]; i++) {
      GetScalar();
    }
  }
  FML_LOG(ERROR) << "Remaining ops: " << (ops_end_ - ops_it_) << ", data: " << (data_end_ - data_it_);
}

void DisplayListRasterizer::Rasterize(SkCanvas* canvas) {
  SkPaint paint;
  paint.setAntiAlias(true);
  FML_LOG(ERROR) << "Starting ops: " << (ops_end_ - ops_it_) << ", data: " << (data_end_ - data_it_);
  while (HasOp()) {
    FML_LOG(INFO) << DescribeNextOp();
    CanvasOp op = GetOp();
    switch (op) {
      case cops_setAA: paint.setAntiAlias(true); break;
      case cops_clearAA: paint.setAntiAlias(false); break;
      case cops_setDither: paint.setDither(true); break;
      case cops_clearDither: paint.setDither(false); break;
      case cops_setInvertColors: break; // TODO(flar): replaces colorfilter???
      case cops_clearInvertColors: paint.setColorFilter(nullptr); break;
      case cops_setColor: paint.setColor(GetColor()); break;
      case cops_setFillStyle: paint.setStyle(SkPaint::Style::kFill_Style); break;
      case cops_setStrokeStyle: paint.setStyle(SkPaint::Style::kStroke_Style); break;
      case cops_setStrokeWidth: paint.setStrokeWidth(GetScalar()); break;
      case cops_setMiterLimit: paint.setStrokeMiter(GetScalar()); break;
      case cops_setCapsButt: paint.setStrokeCap(SkPaint::Cap::kButt_Cap); break;
      case cops_setCapsRound: paint.setStrokeCap(SkPaint::Cap::kRound_Cap); break;
      case cops_setCapsSquare: paint.setStrokeCap(SkPaint::Cap::kSquare_Cap); break;
      case cops_setJoinsMiter: paint.setStrokeJoin(SkPaint::Join::kMiter_Join); break;
      case cops_setJoinsRound: paint.setStrokeJoin(SkPaint::Join::kRound_Join); break;
      case cops_setJoinsBevel: paint.setStrokeJoin(SkPaint::Join::kBevel_Join); break;
      case cops_clearShader: paint.setShader(nullptr); break;
      case cops_setShader: break; // TODO(flar) deal with Shader object
      case cops_clearMaskFilter: paint.setMaskFilter(nullptr); break;
      case cops_setMaskFilterInner: paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kInner_SkBlurStyle, GetScalar())); break;
      case cops_setMaskFilterOuter: paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kOuter_SkBlurStyle, GetScalar())); break;
      case cops_setMaskFilterSolid: paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kSolid_SkBlurStyle, GetScalar())); break;
      case cops_setMaskFilterNormal: paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::kNormal_SkBlurStyle, GetScalar())); break;
      case cops_clearColorFilter: paint.setColorFilter(nullptr); break;
      case cops_setColorFilter: break; // TODO(flar) deal with Filter object
      case cops_clearImageFilter: paint.setImageFilter(nullptr); break;
      case cops_setImageFilter: break; // TODO(flar) deal with Filter object
      case cops_setFilterQualityNearest: paint.setFilterQuality(SkFilterQuality::kNone_SkFilterQuality); break;
      case cops_setFilterQualityLinear: paint.setFilterQuality(SkFilterQuality::kLow_SkFilterQuality); break;
      case cops_setFilterQualityMipmap: paint.setFilterQuality(SkFilterQuality::kMedium_SkFilterQuality); break;
      case cops_setFilterQualityCubic: paint.setFilterQuality(SkFilterQuality::kHigh_SkFilterQuality); break;
      case cops_setBlendMode: paint.setBlendMode(GetBlendMode()); break;

      case cops_save: canvas->save(); break;
      case cops_saveLayer: canvas->saveLayer(nullptr, &paint); break;
      case cops_saveLayerBounds: canvas->saveLayer(GetRect(), &paint); break;
      case cops_restore: canvas->restore(); break;

      case cops_clipRect: canvas->clipRect(GetRect()); break;
      case cops_clipRectAA: canvas->clipRect(GetRect(), true); break;
      case cops_clipRectDiff: canvas->clipRect(GetRect(), SkClipOp::kDifference); break;
      case cops_clipRectAADiff: canvas->clipRect(GetRect(), SkClipOp::kDifference, true); break;

      case cops_clipRRect: canvas->clipRRect(GetRoundRect()); break;
      case cops_clipRRectAA: canvas->clipRRect(GetRoundRect(), true); break;
      case cops_clipPath: break; // TODO(flar) deal with Path object
      case cops_clipPathAA: break; // TODO(flar) deal with Path object

      case cops_translate: canvas->translate(GetScalar(), GetScalar()); break;
      case cops_scale: canvas->scale(GetScalar(), GetScalar()); break;
      case cops_rotate: canvas->rotate(GetScalar() * 180 / M_PI); break;
      case cops_skew: canvas->skew(GetScalar(), GetScalar()); break;
      case cops_transform: break; // TODO(flar) deal with Float64List

      case cops_drawColor: canvas->drawColor(GetColor(), paint.getBlendMode()); break;
      case cops_drawPaint: canvas->drawPaint(paint); break;

      case cops_drawRect: canvas->drawRect(GetRect(), paint); break;
      case cops_drawOval: canvas->drawOval(GetRect(), paint); break;
      case cops_drawRRect: canvas->drawRRect(GetRoundRect(), paint); break;
      case cops_drawDRRect: canvas->drawDRRect(GetRoundRect(), GetRoundRect(), paint); break;
      case cops_drawCircle: canvas->drawCircle(GetPoint(), GetScalar(), paint); break;
      case cops_drawArc: canvas->drawArc(GetRect(), GetScalar(), GetScalar(), false, paint);
      case cops_drawArcCenter: canvas->drawArc(GetRect(), GetScalar(), GetScalar(), true, paint);
      case cops_drawLine: canvas->drawLine(GetPoint(), GetPoint(), paint); break;
      case cops_drawPath: break; // TODO(flar) deal with Path object

      case cops_drawLines: break; // TODO(flar) deal with List of points
      case cops_drawPoints: break; // TODO(flar) deal with List of points
      case cops_drawPolygon: break; // TODO(flar) deal with List of points
      case cops_drawPicture: break; // TODO(flar) deal with Picture object

      case cops_drawImage: GetScalar(); GetScalar(); break; // TODO(flar) deal with image object
      case cops_drawImageRect: GetRect(); GetRect(); break; // TODO(flar) deal with image object
      case cops_drawImageNine: GetRect(); GetRect(); break; // TODO(flar) deal with image object
      case cops_drawAtlas:
      case cops_drawAtlasColored:
        break;
      case cops_drawAtlasCulled:
      case cops_drawAtlasColoredCulled:
        GetRect();
        break;

      case cops_drawParagraph: GetPoint(); break; // TODO(flar) deal with Paragraph object
      case cops_drawShadow: GetScalar(); break; // TODO(flar) deal with Path object
      case cops_drawShadowOccluded: GetScalar(); break; // TODO(flar) deal with Path object
    }
  }
  FML_LOG(ERROR) << "Remaining ops: " << (ops_end_ - ops_it_) << ", data: " << (data_end_ - data_it_);
}

// Dart_Handle DisplayList::toImage(uint32_t width,
//                              uint32_t height,
//                              Dart_Handle raw_image_callback) {
//   if (!picture_.get()) {
//     return tonic::ToDart("Picture is null");
//   }

//   return RasterizeToImage(picture_.get(), width, height, raw_image_callback);
// }

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

// Dart_Handle DisplayList::RasterizeToImage(sk_sp<SkPicture> picture,
//                                       uint32_t width,
//                                       uint32_t height,
//                                       Dart_Handle raw_image_callback) {
//   if (Dart_IsNull(raw_image_callback) || !Dart_IsClosure(raw_image_callback)) {
//     return tonic::ToDart("Image callback was invalid");
//   }

//   if (width == 0 || height == 0) {
//     return tonic::ToDart("Image dimensions for scene were invalid.");
//   }

//   auto* dart_state = UIDartState::Current();
//   auto image_callback = std::make_unique<tonic::DartPersistentValue>(
//       dart_state, raw_image_callback);
//   auto unref_queue = dart_state->GetSkiaUnrefQueue();
//   auto ui_task_runner = dart_state->GetTaskRunners().GetUITaskRunner();
//   auto raster_task_runner = dart_state->GetTaskRunners().GetRasterTaskRunner();
//   auto snapshot_delegate = dart_state->GetSnapshotDelegate();

//   // We can't create an image on this task runner because we don't have a
//   // graphics context. Even if we did, it would be slow anyway. Also, this
//   // thread owns the sole reference to the layer tree. So we flatten the layer
//   // tree into a picture and use that as the thread transport mechanism.

//   auto picture_bounds = SkISize::Make(width, height);

//   auto ui_task = fml::MakeCopyable([image_callback = std::move(image_callback),
//                                     unref_queue](
//                                        sk_sp<SkImage> raster_image) mutable {
//     auto dart_state = image_callback->dart_state().lock();
//     if (!dart_state) {
//       // The root isolate could have died in the meantime.
//       return;
//     }
//     tonic::DartState::Scope scope(dart_state);

//     if (!raster_image) {
//       tonic::DartInvoke(image_callback->Get(), {Dart_Null()});
//       return;
//     }

//     auto dart_image = CanvasImage::Create();
//     dart_image->set_image({std::move(raster_image), std::move(unref_queue)});
//     auto* raw_dart_image = tonic::ToDart(std::move(dart_image));

//     // All done!
//     tonic::DartInvoke(image_callback->Get(), {raw_dart_image});

//     // image_callback is associated with the Dart isolate and must be deleted
//     // on the UI thread.
//     image_callback.reset();
//   });

//   // Kick things off on the raster rask runner.
//   fml::TaskRunner::RunNowOrPostTask(
//       raster_task_runner,
//       [ui_task_runner, snapshot_delegate, picture, picture_bounds, ui_task] {
//         sk_sp<SkImage> raster_image =
//             snapshot_delegate->MakeRasterSnapshot(picture, picture_bounds);

//         fml::TaskRunner::RunNowOrPostTask(
//             ui_task_runner,
//             [ui_task, raster_image]() { ui_task(raster_image); });
//       });

//   return Dart_Null();
// }

}  // namespace flutter
