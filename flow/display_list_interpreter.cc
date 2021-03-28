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

#define CANVAS_OP_MAKE_STRING(name, count, imask, objcount) #name,
#define CANVAS_OP_MAKE_COUNT(name, count, imask, objcount) count,
#define CANVAS_OP_MAKE_IMASK(name, count, imask, objcount) imask,
#define CANVAS_OP_MAKE_OBJCOUNT(name, count, imask, objcount) objcount,

const std::vector<std::string> DisplayListInterpreter::opNames = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_STRING)
};

const std::vector<int> DisplayListInterpreter::opArgCounts = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_COUNT)
};

const std::vector<int> DisplayListInterpreter::opArgImask = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_IMASK)
};

const std::vector<int> DisplayListInterpreter::opObjCounts = {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_OBJCOUNT)
};

DisplayListInterpreter::DisplayListInterpreter(
    std::shared_ptr<std::vector<uint8_t>> ops_vector,
    std::shared_ptr<std::vector<float>> data_vector)
    : ops_it_(ops_vector->begin()),
      ops_end_(ops_vector->end()),
      data_it_(data_vector->begin()),
      data_end_(data_vector->end()),
      ops_vector_(std::move(ops_vector)),
      data_vector_(std::move(data_vector)) {}

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

std::string DisplayListInterpreter::DescribeNextOp() {
  if (!HasOp()) {
    return "END-OF-LIST";
  }
  std::stringstream ss;
  int op_index = *ops_it_;
  ss << opNames[op_index] << "(" << std::hex;
  for (int i = 0; i < opArgCounts[op_index]; i++) {
    if (i > 0) {
      ss << ", ";
    }
    if (data_it_ + i < data_end_) {
      union { float f; uint32_t i; } data;
      data.f = data_it_[i];
      if ((opArgImask[op_index] & (1 << i)) == 0) {
        ss << data.f;
      } else {
        ss << "0x" << data.i;
      }
    } else {
      ss << "... UNDEFINED ...";
      break;
    }
  }
  if (opObjCounts[op_index] > 0) {
    if (opArgCounts[op_index] > 0) {
      ss << ", ";
    }
    if (opObjCounts[op_index] > 1) {
      ss << "[" << opObjCounts[op_index] << " objects]";
    } else {
      ss << "[object]";
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

#define CANVAS_OP_DISPATCH_OP(name, count, imask, objcount) \
  case cops_##name: { execute_##name(context); } break;

void DisplayListInterpreter::Rasterize(SkCanvas* canvas) {
  int entrySaveCount = canvas->getSaveCount();
  RasterizeContext context;
  context.canvas = canvas;
  while (HasOp()) {
    FML_LOG(INFO) << DescribeNextOp();
    switch (GetOp()) {
      FOR_EACH_CANVAS_OP(CANVAS_OP_DISPATCH_OP)
    }
  }
  if (ops_it_ != ops_end_ || data_it_ != data_end_ || canvas->getSaveCount() != entrySaveCount) {
    FML_LOG(ERROR) << "Starting ops: " << ops_vector_->size() << ", data: " << data_vector_->size();
    FML_LOG(ERROR) << "Remaining ops: " << (ops_end_ - ops_it_)
      << ", data: " << (data_end_ - data_it_)
      << ", save count delta: " << (canvas->getSaveCount() - entrySaveCount);
  }
}

#define CANVAS_OP_DEFINE_OP(name, body)                                   \
void DisplayListInterpreter::execute_##name(RasterizeContext& context) {  \
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
CANVAS_OP_DEFINE_OP(setColor, context.paint.setColor(GetColor());)
CANVAS_OP_DEFINE_OP(setFillStyle, context.paint.setStyle(SkPaint::Style::kFill_Style);)
CANVAS_OP_DEFINE_OP(setStrokeStyle, context.paint.setStyle(SkPaint::Style::kStroke_Style);)
CANVAS_OP_DEFINE_OP(setStrokeWidth, context.paint.setStrokeWidth(GetScalar());)
CANVAS_OP_DEFINE_OP(setMiterLimit, context.paint.setStrokeMiter(GetScalar());)

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
  context.paint.setMaskFilter(SkMaskFilter::MakeBlur(SkBlurStyle::k##type##_SkBlurStyle, GetScalar())); \
)
CANVAS_MASK_DEFINE_OP(Inner)
CANVAS_MASK_DEFINE_OP(Outer)
CANVAS_MASK_DEFINE_OP(Solid)
CANVAS_MASK_DEFINE_OP(Normal)

CANVAS_OP_DEFINE_OP(clearImageFilter, context.paint.setImageFilter(nullptr);)
CANVAS_OP_DEFINE_OP(setImageFilter, /* TODO(flar) deal with Filter object */)

#define CANVAS_FQ_DEFINE_OP(op_type, enum_type) CANVAS_OP_DEFINE_OP(setFilterQuality##op_type, \
  context.paint.setFilterQuality(SkFilterQuality::k##enum_type##_SkFilterQuality); \
)
CANVAS_FQ_DEFINE_OP(Nearest, None)
CANVAS_FQ_DEFINE_OP(Linear, Low)
CANVAS_FQ_DEFINE_OP(Mipmap, Medium)
CANVAS_FQ_DEFINE_OP(Cubic, High)

CANVAS_OP_DEFINE_OP(setBlendMode, context.paint.setBlendMode(GetBlendMode());)

CANVAS_OP_DEFINE_OP(save, context.canvas->save();)
CANVAS_OP_DEFINE_OP(saveLayer, context.canvas->saveLayer(nullptr, &context.paint);)
CANVAS_OP_DEFINE_OP(saveLayerBounds, context.canvas->saveLayer(GetRect(), &context.paint);)
CANVAS_OP_DEFINE_OP(restore, context.canvas->restore();)

CANVAS_OP_DEFINE_OP(clipRect, context.canvas->clipRect(GetRect());)
CANVAS_OP_DEFINE_OP(clipRectAA, context.canvas->clipRect(GetRect(), true);)
CANVAS_OP_DEFINE_OP(clipRectDiff, context.canvas->clipRect(GetRect(), SkClipOp::kDifference);)
CANVAS_OP_DEFINE_OP(clipRectAADiff, context.canvas->clipRect(GetRect(), SkClipOp::kDifference, true);)

CANVAS_OP_DEFINE_OP(clipRRect, context.canvas->clipRRect(GetRoundRect());)
CANVAS_OP_DEFINE_OP(clipRRectAA, context.canvas->clipRRect(GetRoundRect(), true);)
CANVAS_OP_DEFINE_OP(clipPath, /* TODO(flar) deal with Path object */)
CANVAS_OP_DEFINE_OP(clipPathAA, /* TODO(flar) deal with Path object */)

CANVAS_OP_DEFINE_OP(translate, context.canvas->translate(GetScalar(), GetScalar());)
CANVAS_OP_DEFINE_OP(scale, context.canvas->scale(GetScalar(), GetScalar());)
CANVAS_OP_DEFINE_OP(rotate, context.canvas->rotate(GetAngle());)
CANVAS_OP_DEFINE_OP(skew, context.canvas->skew(GetScalar(), GetScalar());)
CANVAS_OP_DEFINE_OP(transform, /* TODO(flar) deal with Float64List */)

CANVAS_OP_DEFINE_OP(drawColor, context.canvas->drawColor(context.paint.getColor(), context.paint.getBlendMode());)
CANVAS_OP_DEFINE_OP(drawPaint, context.canvas->drawPaint(context.paint);)

CANVAS_OP_DEFINE_OP(drawRect, context.canvas->drawRect(GetRect(), context.paint);)
CANVAS_OP_DEFINE_OP(drawOval, context.canvas->drawOval(GetRect(), context.paint);)
CANVAS_OP_DEFINE_OP(drawRRect, context.canvas->drawRRect(GetRoundRect(), context.paint);)
CANVAS_OP_DEFINE_OP(drawDRRect, context.canvas->drawDRRect(GetRoundRect(), GetRoundRect(), context.paint);)
CANVAS_OP_DEFINE_OP(drawCircle, context.canvas->drawCircle(GetPoint(), GetScalar(), context.paint);)
CANVAS_OP_DEFINE_OP(drawArc, context.canvas->drawArc(GetRect(), GetAngle(), GetAngle(), false, context.paint);)
CANVAS_OP_DEFINE_OP(drawArcCenter, context.canvas->drawArc(GetRect(), GetAngle(), GetAngle(), true, context.paint);)
CANVAS_OP_DEFINE_OP(drawLine, context.canvas->drawLine(GetPoint(), GetPoint(), context.paint);)
CANVAS_OP_DEFINE_OP(drawPath, /* TODO(flar) deal with Path object */)

CANVAS_OP_DEFINE_OP(drawPoints, /* TODO(flar) deal with List of points */)
CANVAS_OP_DEFINE_OP(drawLines, /* TODO(flar) deal with List of points */)
CANVAS_OP_DEFINE_OP(drawPolygon, /* TODO(flar) deal with List of points */)
CANVAS_OP_DEFINE_OP(drawVertices, /* TODO(flar) deal with List of vertices */)

CANVAS_OP_DEFINE_OP(drawImage, GetPoint(); /* TODO(flar) deal with image object */)
CANVAS_OP_DEFINE_OP(drawImageRect, GetRect(); GetRect(); /* TODO(flar) deal with image object */)
CANVAS_OP_DEFINE_OP(drawImageNine, GetRect(); GetRect(); /* TODO(flar) deal with image object */)

CANVAS_OP_DEFINE_OP(drawAtlas, /* TODO(flar) deal with all of the atlas objects */)
CANVAS_OP_DEFINE_OP(drawAtlasColored, /* TODO(flar) deal with all of the atlas objects */)
CANVAS_OP_DEFINE_OP(drawAtlasCulled, GetRect(); /* TODO(flar) deal with all of the atlas objects */)
CANVAS_OP_DEFINE_OP(drawAtlasColoredCulled, GetRect(); /* TODO(flar) deal with all of the atlas objects */)

CANVAS_OP_DEFINE_OP(drawPicture, /* TODO(flar) deal with Picture object */)
CANVAS_OP_DEFINE_OP(drawParagraph, GetPoint(); /* TODO(flar) deal with Paragraph object */)
CANVAS_OP_DEFINE_OP(drawShadow, GetScalar(); /* TODO(flar) deal with Path object */)
CANVAS_OP_DEFINE_OP(drawShadowOccluded, GetScalar(); /* TODO(flar) deal with Path object */)

}  // namespace flutter
