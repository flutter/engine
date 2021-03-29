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

const std::vector<int> DisplayListInterpreter::opArguments = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_TAKE_ARGS)
};

DisplayListInterpreter::DisplayListInterpreter(
    std::shared_ptr<std::vector<uint8_t>> ops_vector,
    std::shared_ptr<std::vector<float>> data_vector)
    : ops_vector_(std::move(ops_vector)),
      data_vector_(std::move(data_vector)) {}

void DisplayListInterpreter::Describe() {
  Iterator it(this);
  FML_LOG(ERROR) << "Starting ops: " << (it.ops_end - it.ops)
                 << ", data: " << (it.data_end - it.data);
  while(it.HasOp()) {
    FML_LOG(ERROR) << DescribeOneOp(it);
  }
  FML_LOG(ERROR) << "Remaining ops: " << (it.ops_end - it.ops)
                 << ", data: " << (it.data_end - it.data);
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
      case int32_list: ss << "[Int32List]"; break;
      case float32_list: ss << "[Float32List]"; break;
      case matrix_row3: ss << "[" << it.GetScalar() << ", " << it.GetScalar() << ", " << it.GetScalar() << "]"; break;
      case image: ss << "[Image]"; break;
      case path: ss << "[Path]"; break;
      case vertices: ss << "[Vertices]"; break;
      case paragraph: ss << "[Paragraph]"; break;
      case picture: ss << "[Picture]"; break;
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
  context.filterMode = LinearSampling.filter;
  context.sampling = LinearSampling;
  int entrySaveCount = canvas->getSaveCount();
  Iterator it(this);
  while (it.HasOp()) {
    FML_LOG(INFO) << DescribeNextOp(it);
    switch (it.GetOp()) {
      FOR_EACH_CANVAS_OP(CANVAS_OP_DISPATCH_OP)
    }
  }
  if (it.ops != it.ops_end || it.data != it.data_end || canvas->getSaveCount() != entrySaveCount) {
    FML_LOG(ERROR) << "Starting ops: " << ops_vector_->size() << ", data: " << data_vector_->size();
    FML_LOG(ERROR) << "Remaining ops: " << (it.ops_end - it.ops)
      << ", data: " << (it.data_end - it.data)
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
CANVAS_OP_DEFINE_OP(setColorFilter, /* TODO(flar) deal with Filter objec */)
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
CANVAS_OP_DEFINE_OP(setShader, /* TODO(flar) deal with Shader object */)

CANVAS_OP_DEFINE_OP(clearMaskFilter, context.paint.setMaskFilter(nullptr);)
#define CANVAS_MASK_DEFINE_OP(type) CANVAS_OP_DEFINE_OP(setMaskFilter##type, \
  context.paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::k##type##_SkBlurStyle, it.GetScalar())); \
)
CANVAS_MASK_DEFINE_OP(Inner)
CANVAS_MASK_DEFINE_OP(Outer)
CANVAS_MASK_DEFINE_OP(Solid)
CANVAS_MASK_DEFINE_OP(Normal)

CANVAS_OP_DEFINE_OP(clearImageFilter, context.paint.setImageFilter(nullptr);)
CANVAS_OP_DEFINE_OP(setImageFilter, /* TODO(flar) deal with Filter object */)

const SkSamplingOptions DisplayListInterpreter::NearestSampling =
  SkSamplingOptions(SkFilterMode::kNearest, SkMipmapMode::kNone);
const SkSamplingOptions DisplayListInterpreter::LinearSampling =
  SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kNone);
const SkSamplingOptions DisplayListInterpreter::MipmapSampling =
  SkSamplingOptions(SkFilterMode::kLinear, SkMipmapMode::kLinear);
const SkSamplingOptions DisplayListInterpreter::CubicSampling =
  SkSamplingOptions(SkCubicResampler{1 / 3.0f, 1 / 3.0f});

#define CANVAS_FQ_DEFINE_OP(op_type, enum_type, filter_mode) \
CANVAS_OP_DEFINE_OP(setFilterQuality##op_type,               \
  context.filterMode = SkFilterMode::filter_mode;            \
  context.sampling = op_type##Sampling;                      \
  context.paint.setFilterQuality(                            \
      SkFilterQuality::k##enum_type##_SkFilterQuality);      \
)
CANVAS_FQ_DEFINE_OP(Nearest, None, kNearest)
CANVAS_FQ_DEFINE_OP(Linear, Low, kLinear)
CANVAS_FQ_DEFINE_OP(Mipmap, Medium, kLinear)
CANVAS_FQ_DEFINE_OP(Cubic, High, kLinear)

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
CANVAS_OP_DEFINE_OP(clipPath, /* TODO(flar) deal with Path object */)
CANVAS_OP_DEFINE_OP(clipPathAA, /* TODO(flar) deal with Path object */)

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
CANVAS_OP_DEFINE_OP(drawPath, /* TODO(flar) deal with Path object */)

CANVAS_OP_DEFINE_OP(drawPoints, /* TODO(flar) deal with List of points */)
CANVAS_OP_DEFINE_OP(drawLines, /* TODO(flar) deal with List of points */)
CANVAS_OP_DEFINE_OP(drawPolygon, /* TODO(flar) deal with List of points */)
CANVAS_OP_DEFINE_OP(drawVertices, /* TODO(flar) deal with List of vertices */)

CANVAS_OP_DEFINE_OP(drawImage, it.GetPoint(); /* TODO(flar) deal with image object */)
CANVAS_OP_DEFINE_OP(drawImageRect, it.GetRect(); it.GetRect(); /* TODO(flar) deal with image object */)
CANVAS_OP_DEFINE_OP(drawImageNine, it.GetRect(); it.GetRect(); /* TODO(flar) deal with image object */)

CANVAS_OP_DEFINE_OP(drawAtlas, /* TODO(flar) deal with all of the atlas objects */)
CANVAS_OP_DEFINE_OP(drawAtlasColored, /* TODO(flar) deal with all of the atlas objects */)
CANVAS_OP_DEFINE_OP(drawAtlasCulled, it.GetRect(); /* TODO(flar) deal with all of the atlas objects */)
CANVAS_OP_DEFINE_OP(drawAtlasColoredCulled, it.GetRect(); /* TODO(flar) deal with all of the atlas objects */)

CANVAS_OP_DEFINE_OP(drawPicture, /* TODO(flar) deal with Picture object */)
CANVAS_OP_DEFINE_OP(drawParagraph, it.GetPoint(); /* TODO(flar) deal with Paragraph object */)
CANVAS_OP_DEFINE_OP(drawShadow, it.GetScalar(); /* TODO(flar) deal with Path object */)
CANVAS_OP_DEFINE_OP(drawShadowOccluded, it.GetScalar(); /* TODO(flar) deal with Path object */)

}  // namespace flutter
