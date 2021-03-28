// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/display_list_interpreter.h"
#include "flutter/fml/logging.h"

#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkShader.h"

namespace flutter {

#define CANVAS_OP_MAKE_STRING(name, count, imask, objcount) #name
#define CANVAS_OP_MAKE_COUNT(name, count, imask, objcount) count
#define CANVAS_OP_MAKE_IMASK(name, count, imask, objcount) imask
#define CANVAS_OP_MAKE_OBJCOUNT(name, count, imask, objcount) objcount

const std::vector<std::string> DisplayListInterpreter::opNames = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_STRING),
};

const std::vector<int> DisplayListInterpreter::opArgCounts = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_COUNT),
};

const std::vector<int> DisplayListInterpreter::opArgImask = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_IMASK),
};

const std::vector<int> DisplayListInterpreter::opObjCounts = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_OBJCOUNT),
};

DisplayListInterpreter::DisplayListInterpreter(
    std::shared_ptr<std::vector<uint8_t>> ops_vector,
    std::shared_ptr<std::vector<float>> data_vector)
    : ops_vector_(ops_vector),
      ops_it_(ops_vector->begin()),
      ops_end_(ops_vector->end()),
      data_vector_(data_vector),
      data_it_(data_vector->begin()),
      data_end_(data_vector->end()) {}

static const std::array<SkSamplingOptions, 4> filter_qualities = {
    SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone),
    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone),
    SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear),
    SkSamplingOptions(SkCubicResampler{1 / 3.0f, 1 / 3.0f}),
};

void DisplayListInterpreter::Describe() {
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

constexpr float invert_colors[20] = {
  -1.0,    0,    0, 1.0, 0,
     0, -1.0,    0, 1.0, 0,
     0,    0, -1.0, 1.0, 0,
   1.0,  1.0,  1.0, 1.0, 0
};

static sk_sp<SkColorFilter> makeColorFilter(bool invertColors, sk_sp<SkColorFilter> filter) {
  if (!invertColors) {
    return filter;
  }
  sk_sp<SkColorFilter> invert_filter = invertColors ? SkColorFilters::Matrix(invert_colors) : nullptr;
  if (filter) {
    invert_filter = invert_filter->makeComposed(filter);
  }
  return invert_filter;
}

void DisplayListInterpreter::Rasterize(SkCanvas* canvas) {
  int entrySaveCount = canvas->getSaveCount();
  SkPaint paint;
  bool invertColors = false;
  sk_sp<SkColorFilter> colorFilter;
  FML_LOG(ERROR) << "Starting ops: " << (ops_end_ - ops_it_) << ", data: " << (data_end_ - data_it_);
  while (HasOp()) {
    FML_LOG(INFO) << DescribeNextOp();
    CanvasOp op = GetOp();
    switch (op) {
      case cops_setAA: paint.setAntiAlias(true); break;
      case cops_clearAA: paint.setAntiAlias(false); break;
      case cops_setDither: paint.setDither(true); break;
      case cops_clearDither: paint.setDither(false); break;
      case cops_setInvertColors: invertColors = true; paint.setColorFilter(makeColorFilter(invertColors, colorFilter)); break;
      case cops_clearInvertColors: invertColors = false; paint.setColorFilter(colorFilter); break;
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
      case cops_clearColorFilter: colorFilter = nullptr; paint.setColorFilter(makeColorFilter(invertColors, colorFilter)); break;
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
      case cops_rotate: canvas->rotate(GetAngle()); break;
      case cops_skew: canvas->skew(GetScalar(), GetScalar()); break;
      case cops_transform: break; // TODO(flar) deal with Float64List

      case cops_drawColor: canvas->drawColor(paint.getColor(), paint.getBlendMode()); break;
      case cops_drawPaint: canvas->drawPaint(paint); break;

      case cops_drawRect: canvas->drawRect(GetRect(), paint); break;
      case cops_drawOval: canvas->drawOval(GetRect(), paint); break;
      case cops_drawRRect: canvas->drawRRect(GetRoundRect(), paint); break;
      case cops_drawDRRect: canvas->drawDRRect(GetRoundRect(), GetRoundRect(), paint); break;
      case cops_drawCircle: canvas->drawCircle(GetPoint(), GetScalar(), paint); break;
      case cops_drawArc: canvas->drawArc(GetRect(), GetAngle(), GetAngle(), false, paint); break;
      case cops_drawArcCenter: canvas->drawArc(GetRect(), GetAngle(), GetAngle(), true, paint); break;
      case cops_drawLine: canvas->drawLine(GetPoint(), GetPoint(), paint); break;
      case cops_drawPath: break; // TODO(flar) deal with Path object

      case cops_drawPoints: break; // TODO(flar) deal with List of points
      case cops_drawLines: break; // TODO(flar) deal with List of points
      case cops_drawPolygon: break; // TODO(flar) deal with List of points
      case cops_drawVertices: break; // TODO(flar) deal with List of vertices

      case cops_drawImage: GetPoint(); break; // TODO(flar) deal with image object
      case cops_drawImageRect: GetRect(); GetRect(); break; // TODO(flar) deal with image object
      case cops_drawImageNine: GetRect(); GetRect(); break; // TODO(flar) deal with image object
      case cops_drawAtlas:
      case cops_drawAtlasColored:
        break;
      case cops_drawAtlasCulled:
      case cops_drawAtlasColoredCulled:
        GetRect();
        break;

      case cops_drawPicture: break; // TODO(flar) deal with Picture object
      case cops_drawParagraph: GetPoint(); break; // TODO(flar) deal with Paragraph object
      case cops_drawShadow: GetScalar(); break; // TODO(flar) deal with Path object
      case cops_drawShadowOccluded: GetScalar(); break; // TODO(flar) deal with Path object
    }
  }
  FML_LOG(ERROR) << "Remaining ops: " << (ops_end_ - ops_it_)
    << ", data: " << (data_end_ - data_it_)
    << ", save count delta: " << (canvas->getSaveCount() - entrySaveCount);
}

}  // namespace flutter
