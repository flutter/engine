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

#define CANVAS_OP_TAKE_STRING(name, arg) #name,
#define CANVAS_OP_TAKE_ARGS(name, args) CANVAS_OP_ARGS_##args,

const std::vector<std::string> DisplayListInterpreter::opNames = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_TAKE_STRING)
};

const std::vector<uint32_t> DisplayListInterpreter::opArguments = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_TAKE_ARGS)
};

DisplayListInterpreter::DisplayListInterpreter(DisplayListData data)
    : ops_vector_(data.ops_vector),
      data_vector_(data.data_vector),
      ref_vector_(data.ref_vector) {}

DisplayListInterpreter::DisplayListInterpreter(
    std::shared_ptr<std::vector<uint8_t>> ops_vector,
    std::shared_ptr<std::vector<float>> data_vector,
    std::shared_ptr<std::vector<DisplayListRefHolder>> ref_vector)
    : ops_vector_(std::move(ops_vector)),
      data_vector_(std::move(data_vector)),
      ref_vector_(std::move(ref_vector)) {}

void DisplayListInterpreter::Describe() {
  Iterator it(this);
  FML_LOG(ERROR) << "Starting ops: " << (it.ops_end - it.ops)
                 << ", data: " << (it.data_end - it.data)
                 << ", refs: " << (it.refs_end - it.refs);
  while(it.HasOp()) {
    FML_LOG(ERROR) << DescribeOneOp(it);
  }
  FML_LOG(ERROR) << "Remaining ops: " << (it.ops_end - it.ops)
                 << ", data: " << (it.data_end - it.data)
                 << ", refs: " << (it.refs_end - it.refs);
}

std::string DisplayListInterpreter::DescribeNextOp(const Iterator& it) {
  Iterator it_copy(it);
  return DescribeOneOp(it_copy);
}

std::string DisplayListInterpreter::DescribeOneOp(Iterator& it) {
  if (!it.HasOp()) {
    return "END-OF-LIST";
  }
  std::stringstream ss;
  CanvasOp op = it.GetOp();
  ss << opNames[op] << "(";
  bool first = true;
  for (uint32_t arg_types = opArguments[op]; arg_types != 0; arg_types >>= CANVAS_OP_ARG_SHIFT) {
    if (first) {
      first = false;
    } else {
      ss << ", ";
    }
    CanvasOpArg arg_type = static_cast<CanvasOpArg>(arg_types & CANVAS_OP_ARG_MASK);
    switch (arg_type) {
      case empty: ss << "?none?"; break; // This should never happen
      case scalar: ss << it.GetScalar(); break;
      case color: ss << "Color(" << std::hex << "0x" << it.GetUint32() << ")"; break;
      case blend_mode: ss << "BlendMode(" << std::dec << it.GetUint32() << ")"; break;
      case angle: ss << it.GetAngle(); break;
      case point: ss << "Point(x: " << it.GetScalar() << ", y: " << it.GetScalar() << ")"; break;
      case rect: ss << "Rect(l: " << it.GetScalar() << ", t: " << it.GetScalar()
                    << ", r: " << it.GetScalar() << ", b: " << it.GetScalar() << ")"; break;
      case round_rect: ss << "RoundRect("
                          << "Rect(l: " << it.GetScalar() << ", t: " << it.GetScalar() << ", r: "
                                        << it.GetScalar() << ", b: " << it.GetScalar() << "), "
                          << "ul: (h: " << it.GetScalar() << ", v: " << it.GetScalar() << "), "
                          << "ur: (h: " << it.GetScalar() << ", v: " << it.GetScalar() << "), "
                          << "lr: (h: " << it.GetScalar() << ", v: " << it.GetScalar() << "), "
                          << "ll: (h: " << it.GetScalar() << ", v: " << it.GetScalar() << "))"; break;
      case uint32_list:
      case scalar_list: {
        uint32_t len = it.GetUint32();
        ss << std::dec << "(len=" << len << ")[" << std::hex;
        for (uint32_t i = 0; i < len; i++) {
          if (i > 0) {
            ss << ", ";
            if (len > 8 && i == 4) {
              it.skipData(len - 8);
              i += (len - 8);
              ss << "..., ";
            }
          }
          if (arg_type == scalar_list) {
            ss << it.GetScalar();
          } else {
            ss << it.GetUint32();
          }
        }
        ss << "]";
        break;
      }
      case matrix_row3: ss << "[" << it.GetScalar() << ", " << it.GetScalar() << ", " << it.GetScalar() << "]"; break;
      case image: ss << "[Image]"; break;
      case path: ss << "[Path]"; break;
      case vertices: ss << "[Vertices]"; break;
      case skpicture: ss << "[SkPicture]"; break;
      case display_list: ss << "[DisplayList]"; break;
      case shader: ss << "[Shader]"; break;
      case color_filter: ss << "[ColorFilter]"; break;
      case image_filter: ss << "[ImageFilter]"; break;
    }
  }
  ss << ")";
  return ss.str();
}

constexpr float invert_colors[20] = {
  -1.0,    0,    0, 1.0, 0,
     0, -1.0,    0, 1.0, 0,
     0,    0, -1.0, 1.0, 0,
   1.0,  1.0,  1.0, 1.0, 0
};

sk_sp<SkColorFilter> DisplayListInterpreter::makeColorFilter(RasterizeContext& context) {
  if (!context.invertColors) {
    return context.colorFilter;
  }
  sk_sp<SkColorFilter> invert_filter = SkColorFilters::Matrix(invert_colors);
  if (context.colorFilter) {
    invert_filter = invert_filter->makeComposed(context.colorFilter);
  }
  return invert_filter;
}

#define CANVAS_OP_DISPATCH_OP(name, args) \
  case cops_##name: { execute_##name(context, it); } break;

void DisplayListInterpreter::Rasterize(SkCanvas* canvas) {
  RasterizeContext context;
  context.canvas = canvas;
  context.filterMode = NearestSampling.filter;
  context.sampling = NearestSampling;
  int entrySaveCount = canvas->getSaveCount();
  Iterator it(this);
  while (it.HasOp()) {
    FML_LOG(INFO) << DescribeNextOp(it);
    switch (it.GetOp()) {
      FOR_EACH_CANVAS_OP(CANVAS_OP_DISPATCH_OP)
    }
  }
  if (it.ops != it.ops_end ||
      it.data != it.data_end ||
      it.refs != it.refs_end ||
      canvas->getSaveCount() != entrySaveCount) {
    FML_LOG(ERROR) << "Starting ops: " << ops_vector_->size()
                   << ", data: " << data_vector_->size()
                   << ", refs: " << ref_vector_->size();
    FML_LOG(ERROR) << "Remaining ops: " << (it.ops_end - it.ops)
      << ", data: " << (it.data_end - it.data)
      << ", refs: " << (it.refs_end - it.refs)
      << ", save count delta: " << (canvas->getSaveCount() - entrySaveCount);
  }
}

#define CANVAS_OP_DEFINE_OP(name, body)                                   \
void DisplayListInterpreter::execute_##name(RasterizeContext& context, Iterator& it) {  \
  body                                                                    \
}

CANVAS_OP_DEFINE_OP(setAA, context.paint.setAntiAlias(true);)
CANVAS_OP_DEFINE_OP(clearAA, context.paint.setAntiAlias(false);)
CANVAS_OP_DEFINE_OP(setDither, context.paint.setDither(true);)
CANVAS_OP_DEFINE_OP(clearDither, context.paint.setDither(false);)

CANVAS_OP_DEFINE_OP(setInvertColors,
  context.invertColors = true;
  context.paint.setColorFilter(makeColorFilter(context));
)
CANVAS_OP_DEFINE_OP(clearInvertColors,
  context.invertColors = false;
  context.paint.setColorFilter(context.colorFilter);
)
CANVAS_OP_DEFINE_OP(clearColorFilter,
  context.colorFilter = nullptr;
  context.paint.setColorFilter(makeColorFilter(context));
)
CANVAS_OP_DEFINE_OP(setColorFilter,
  context.colorFilter = it.GetColorFilter();
  context.paint.setColorFilter(makeColorFilter(context));
)
CANVAS_OP_DEFINE_OP(setColor, context.paint.setColor(it.GetColor());)
CANVAS_OP_DEFINE_OP(setFillStyle, context.paint.setStyle(SkPaint::Style::kFill_Style);)
CANVAS_OP_DEFINE_OP(setStrokeStyle, context.paint.setStyle(SkPaint::Style::kStroke_Style);)
CANVAS_OP_DEFINE_OP(setStrokeWidth, context.paint.setStrokeWidth(it.GetScalar());)
CANVAS_OP_DEFINE_OP(setMiterLimit, context.paint.setStrokeMiter(it.GetScalar());)

#define CANVAS_CAP_DEFINE_OP(cap) CANVAS_OP_DEFINE_OP(setCaps##cap, \
  context.paint.setStrokeCap(SkPaint::Cap::k##cap##_Cap); \
)
CANVAS_CAP_DEFINE_OP(Butt)
CANVAS_CAP_DEFINE_OP(Round)
CANVAS_CAP_DEFINE_OP(Square)

#define CANVAS_JOIN_DEFINE_OP(join) CANVAS_OP_DEFINE_OP(setJoins##join, \
  context.paint.setStrokeJoin(SkPaint::Join::k##join##_Join); \
)
CANVAS_JOIN_DEFINE_OP(Miter)
CANVAS_JOIN_DEFINE_OP(Round)
CANVAS_JOIN_DEFINE_OP(Bevel)

CANVAS_OP_DEFINE_OP(clearShader, context.paint.setShader(nullptr);)
CANVAS_OP_DEFINE_OP(setShader, context.paint.setShader(it.GetShader());)

CANVAS_OP_DEFINE_OP(clearMaskFilter, context.paint.setMaskFilter(nullptr);)
#define CANVAS_MASK_DEFINE_OP(type) CANVAS_OP_DEFINE_OP(setMaskFilter##type, \
  context.paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::k##type##_SkBlurStyle, it.GetScalar())); \
)
CANVAS_MASK_DEFINE_OP(Inner)
CANVAS_MASK_DEFINE_OP(Outer)
CANVAS_MASK_DEFINE_OP(Solid)
CANVAS_MASK_DEFINE_OP(Normal)

CANVAS_OP_DEFINE_OP(clearImageFilter, context.paint.setImageFilter(nullptr);)
CANVAS_OP_DEFINE_OP(setImageFilter, context.paint.setImageFilter(it.GetImageFilter());)

const SkSamplingOptions DisplayListInterpreter::NearestSampling =
  SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone);
const SkSamplingOptions DisplayListInterpreter::LinearSampling =
  SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
const SkSamplingOptions DisplayListInterpreter::MipmapSampling =
  SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
const SkSamplingOptions DisplayListInterpreter::CubicSampling =
  SkSamplingOptions(SkCubicResampler{1 / 3.0f, 1 / 3.0f});

#define CANVAS_OP_DEFINE_FQ(op_type, enum_type, filter_mode) \
CANVAS_OP_DEFINE_OP(setFilterQuality##op_type,               \
  context.filterMode = SkFilterMode::filter_mode;            \
  context.sampling = op_type##Sampling;                      \
  context.paint.setFilterQuality(                            \
      SkFilterQuality::k##enum_type##_SkFilterQuality);      \
)
CANVAS_OP_DEFINE_FQ(Nearest, None, kNearest)
CANVAS_OP_DEFINE_FQ(Linear, Low, kLinear)
CANVAS_OP_DEFINE_FQ(Mipmap, Medium, kLinear)
CANVAS_OP_DEFINE_FQ(Cubic, High, kLinear)

CANVAS_OP_DEFINE_OP(setBlendMode, context.paint.setBlendMode(it.GetBlendMode());)

CANVAS_OP_DEFINE_OP(save, context.canvas->save();)
CANVAS_OP_DEFINE_OP(saveLayer, context.canvas->saveLayer(nullptr, &context.paint);)
CANVAS_OP_DEFINE_OP(saveLayerBounds, context.canvas->saveLayer(it.GetRect(), &context.paint);)
CANVAS_OP_DEFINE_OP(restore, context.canvas->restore();)

CANVAS_OP_DEFINE_OP(clipRect, context.canvas->clipRect(it.GetRect());)
CANVAS_OP_DEFINE_OP(clipRectAA, context.canvas->clipRect(it.GetRect(), true);)
CANVAS_OP_DEFINE_OP(clipRectDiff, context.canvas->clipRect(it.GetRect(), SkClipOp::kDifference);)
CANVAS_OP_DEFINE_OP(clipRectAADiff, context.canvas->clipRect(it.GetRect(), SkClipOp::kDifference, true);)

CANVAS_OP_DEFINE_OP(clipRRect, context.canvas->clipRRect(it.GetRoundRect());)
CANVAS_OP_DEFINE_OP(clipRRectAA, context.canvas->clipRRect(it.GetRoundRect(), true);)
CANVAS_OP_DEFINE_OP(clipPath, it.skipSkRef(); /* TODO(flar) deal with Path object */)
CANVAS_OP_DEFINE_OP(clipPathAA, it.skipSkRef(); /* TODO(flar) deal with Path object */)

CANVAS_OP_DEFINE_OP(translate, context.canvas->translate(it.GetScalar(), it.GetScalar());)
CANVAS_OP_DEFINE_OP(scale, context.canvas->scale(it.GetScalar(), it.GetScalar());)
CANVAS_OP_DEFINE_OP(rotate, context.canvas->rotate(it.GetAngle());)
CANVAS_OP_DEFINE_OP(skew, context.canvas->skew(it.GetScalar(), it.GetScalar());)
CANVAS_OP_DEFINE_OP(transform2x3,
  context.canvas->concat(SkMatrix::MakeAll(
    it.GetScalar(), it.GetScalar(), it.GetScalar(),
    it.GetScalar(), it.GetScalar(), it.GetScalar(),
    0.0, 0.0, 0.1
  ));
)
CANVAS_OP_DEFINE_OP(transform3x3,
  context.canvas->concat(SkMatrix::MakeAll(
    it.GetScalar(), it.GetScalar(), it.GetScalar(),
    it.GetScalar(), it.GetScalar(), it.GetScalar(),
    it.GetScalar(), it.GetScalar(), it.GetScalar()
  ));
)

CANVAS_OP_DEFINE_OP(drawColor, context.canvas->drawColor(context.paint.getColor(), context.paint.getBlendMode());)
CANVAS_OP_DEFINE_OP(drawPaint, context.canvas->drawPaint(context.paint);)

CANVAS_OP_DEFINE_OP(drawRect, context.canvas->drawRect(it.GetRect(), context.paint);)
CANVAS_OP_DEFINE_OP(drawOval, context.canvas->drawOval(it.GetRect(), context.paint);)
CANVAS_OP_DEFINE_OP(drawRRect, context.canvas->drawRRect(it.GetRoundRect(), context.paint);)
CANVAS_OP_DEFINE_OP(drawDRRect, context.canvas->drawDRRect(it.GetRoundRect(), it.GetRoundRect(), context.paint);)
CANVAS_OP_DEFINE_OP(drawCircle, context.canvas->drawCircle(it.GetPoint(), it.GetScalar(), context.paint);)
CANVAS_OP_DEFINE_OP(drawArc, context.canvas->drawArc(it.GetRect(), it.GetAngle(), it.GetAngle(), false, context.paint);)
CANVAS_OP_DEFINE_OP(drawArcCenter, context.canvas->drawArc(it.GetRect(), it.GetAngle(), it.GetAngle(), true, context.paint);)
CANVAS_OP_DEFINE_OP(drawLine, context.canvas->drawLine(it.GetPoint(), it.GetPoint(), context.paint);)
CANVAS_OP_DEFINE_OP(drawPath, it.skipSkRef(); /* TODO(flar) deal with Path object */)

#define CANVAS_OP_DEFINE_POINT_OP(mode)                                   \
CANVAS_OP_DEFINE_OP(draw##mode,                                           \
  SkScalar *flt_ptr;                                                      \
  uint32_t len = it.GetFloatList(&flt_ptr);                               \
  const SkPoint *pt_ptr = reinterpret_cast<const SkPoint*>(flt_ptr);      \
  context.canvas->drawPoints(SkCanvas::PointMode::k##mode##_PointMode,    \
                             len / 2, pt_ptr, context.paint);             \
)
CANVAS_OP_DEFINE_POINT_OP(Points)
CANVAS_OP_DEFINE_POINT_OP(Lines)
CANVAS_OP_DEFINE_POINT_OP(Polygon)
CANVAS_OP_DEFINE_OP(drawVertices, context.canvas->drawVertices(it.GetVertices(), context.paint.getBlendMode(), context.paint);)

CANVAS_OP_DEFINE_OP(drawImage,
  sk_sp<SkImage> image = it.GetImage();
  SkPoint point = it.GetPoint();
  context.canvas->drawImage(image, point.fX, point.fY, context.sampling, &context.paint);
)
CANVAS_OP_DEFINE_OP(drawImageRect,
  sk_sp<SkImage> image = it.GetImage();
  SkRect src = it.GetRect();
  SkRect dst = it.GetRect();
  context.canvas->drawImageRect(image, src, dst, context.sampling, &context.paint, SkCanvas::kFast_SrcRectConstraint);
)
CANVAS_OP_DEFINE_OP(drawImageNine,
  sk_sp<SkImage> image = it.GetImage();
  SkRect center = it.GetRect();
  SkRect dst = it.GetRect();
  context.canvas->drawImageNine(image.get(), center.round(), dst, context.filterMode);
)

#define CANVAS_OP_DEFINE_ATLAS(op_type, has_colors, has_rect)   \
CANVAS_OP_DEFINE_OP(op_type,                                    \
  sk_sp<SkImage> image = it.GetImage();                         \
  SkScalar *rst_ptr;                                            \
  int nrstscalars = it.GetFloatList(&rst_ptr);                  \
  SkScalar *rect_ptr;                                           \
  int nrectscalars = it.GetFloatList(&rect_ptr);                \
  int numrects = nrectscalars / 4;                              \
  uint32_t *clr_ptr = nullptr;                                  \
  int ncolorints = numrects;                                    \
  if (has_colors) {                                             \
    ncolorints = it.GetIntList(&clr_ptr);                       \
  }                                                             \
  SkRect* pCullRect = nullptr;                                  \
  SkRect cull_rect;                                             \
  if (has_rect) {                                               \
    cull_rect = it.GetRect();                                   \
    pCullRect = &cull_rect;                                     \
  }                                                             \
  if (nrectscalars != numrects * 4 ||                           \
      nrstscalars != numrects * 4 ||                            \
      ncolorints != numrects) {                                 \
    FML_LOG(ERROR) << "Mismatched Atlas array lengths";         \
    return;                                                     \
  }                                                             \
  context.canvas->drawAtlas(                                    \
    image.get(),                                                \
    reinterpret_cast<const SkRSXform*>(rst_ptr),                \
    reinterpret_cast<const SkRect*>(rect_ptr),                  \
    reinterpret_cast<const SkColor*>(clr_ptr),                  \
    numrects,                                                   \
    context.paint.getBlendMode(), context.sampling, pCullRect,  \
    &context.paint);                                            \
)
CANVAS_OP_DEFINE_ATLAS(drawAtlas, false, false)
CANVAS_OP_DEFINE_ATLAS(drawAtlasColored, true, false)
CANVAS_OP_DEFINE_ATLAS(drawAtlasCulled, false, true)
CANVAS_OP_DEFINE_ATLAS(drawAtlasColoredCulled, true, true)

CANVAS_OP_DEFINE_OP(drawDisplayList, DisplayListInterpreter(it.GetDisplayList()).Rasterize(context.canvas);)
CANVAS_OP_DEFINE_OP(drawSkPicture, context.canvas->drawPicture(it.GetSkPicture());)
CANVAS_OP_DEFINE_OP(drawShadow, it.skipSkRef(); it.GetScalar(); /* TODO(flar) deal with Path object */)
CANVAS_OP_DEFINE_OP(drawShadowOccluded, it.skipSkRef(); it.GetScalar(); /* TODO(flar) deal with Path object */)

}  // namespace flutter
