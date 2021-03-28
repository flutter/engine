// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_INTERPRETER_H_
#define FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_INTERPRETER_H_

#include <sstream>

#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkRRect.h"

namespace flutter {

// opName (matches Dart enum name), numDataArgs, intArgMask, numObjArgs
#define FOR_EACH_CANVAS_OP(V) \
  V(setAA, 0, 0, 0)                   \
  V(clearAA, 0, 0, 0)                 \
  V(setDither, 0, 0, 0)               \
  V(clearDither, 0, 0, 0)             \
  V(setInvertColors, 0, 0, 0)         \
  V(clearInvertColors, 0, 0, 0)       \
  V(setFillStyle, 0, 0, 0)            \
  V(setStrokeStyle, 0, 0, 0)          \
                                      \
  V(setCapsButt, 0, 0, 0)             \
  V(setCapsRound, 0, 0, 0)            \
  V(setCapsSquare, 0, 0, 0)           \
  V(setJoinsBevel, 0, 0, 0)           \
  V(setJoinsMiter, 0, 0, 0)           \
  V(setJoinsRound, 0, 0, 0)           \
                                      \
  V(setStrokeWidth, 1, 0, 0)          \
  V(setMiterLimit, 1, 0, 0)           \
                                      \
  V(setFilterQualityNearest, 0, 0, 0) \
  V(setFilterQualityLinear, 0, 0, 0)  \
  V(setFilterQualityMipmap, 0, 0, 0)  \
  V(setFilterQualityCubic, 0, 0, 0)   \
                                      \
  V(setColor, 1, 0x1, 0)              \
  V(setBlendMode, 1, 0x1, 0)          \
                                      \
  V(setShader, 0, 0, 1)               \
  V(clearShader, 0, 0, 0)             \
  V(setColorFilter, 0, 0, 1)          \
  V(clearColorFilter, 0, 0, 0)        \
  V(setImageFilter, 0, 0, 1)          \
  V(clearImageFilter, 0, 0, 0)        \
                                      \
  V(clearMaskFilter, 0, 0, 0)         \
  V(setMaskFilterNormal, 1, 0, 0)     \
  V(setMaskFilterSolid, 1, 0, 0)      \
  V(setMaskFilterOuter, 1, 0, 0)      \
  V(setMaskFilterInner, 1, 0, 0)      \
                                      \
  V(save, 0, 0, 0)                    \
  V(saveLayer, 0, 0, 0)               \
  V(saveLayerBounds, 4, 0, 0)         \
  V(restore, 0, 0, 0)                 \
                                      \
  V(translate, 2, 0, 0)               \
  V(scale, 2, 0, 0)                   \
  V(rotate, 1, 0, 0)                  \
  V(skew, 2, 0, 0)                    \
  V(transform, 0, 0, 1)               \
                                      \
  V(clipRect, 4, 0, 0)                \
  V(clipRectAA, 4, 0, 0)              \
  V(clipRectDiff, 4, 0, 0)            \
  V(clipRectAADiff, 4, 0, 0)          \
  V(clipRRect, 12, 0, 0)              \
  V(clipRRectAA, 12, 0, 0)            \
  V(clipPath, 0, 0, 1)                \
  V(clipPathAA, 0, 0, 1)              \
                                      \
  V(drawPaint, 0, 0, 0)               \
  V(drawColor, 2, 0x3, 0)             \
                                      \
  V(drawLine, 4, 0, 0)                \
  V(drawRect, 4, 0, 0)                \
  V(drawOval, 4, 0, 0)                \
  V(drawCircle, 3, 0, 0)              \
  V(drawRRect, 12, 0, 0)              \
  V(drawDRRect, 24, 0, 0)             \
  V(drawArc, 6, 0, 0)                 \
  V(drawArcCenter, 6, 0, 0)           \
  V(drawPath, 0, 0, 1)                \
                                      \
  V(drawPoints, 0, 0, 1)              \
  V(drawLines, 0, 0, 1)               \
  V(drawPolygon, 0, 0, 1)             \
  V(drawVertices, 0, 0, 1)            \
                                      \
  V(drawImage, 2, 0, 1)               \
  V(drawImageRect, 8, 0, 1)           \
  V(drawImageNine, 8, 0, 1)           \
  V(drawAtlas, 0, 0, 3)               \
  V(drawAtlasColored, 0, 0, 4)        \
  V(drawAtlasCulled, 4, 0, 3)         \
  V(drawAtlasColoredCulled, 4, 0, 4)  \
                                      \
  V(drawParagraph, 2, 0, 1)           \
  V(drawPicture, 0, 0, 1)             \
  V(drawShadow, 1, 0, 1)              \
  V(drawShadowOccluded, 1, 0, 1)

#define CANVAS_OP_MAKE_ENUM(name, count, imask, objcount) cops_##name,
enum CanvasOp {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_ENUM)
};

#define CANVAS_OP_DECLARE_OP(name, count, imask, objcount) void execute_##name(RasterizeContext& context);

class DisplayListInterpreter {
 public:
  DisplayListInterpreter(std::shared_ptr<std::vector<uint8_t>> ops,
                         std::shared_ptr<std::vector<float>> data);

  void Rasterize(SkCanvas *canvas);

  void Describe();

  static const std::vector<std::string> opNames;
  static const std::vector<int> opArgCounts;
  static const std::vector<int> opArgImask;
  static const std::vector<int> opObjCounts;

 private:
  std::vector<uint8_t>::iterator ops_it_;
  const std::vector<uint8_t>::iterator ops_end_;
  std::vector<float>::iterator data_it_;
  const std::vector<float>::iterator data_end_;

  std::shared_ptr<std::vector<uint8_t>> ops_vector_;
  std::shared_ptr<std::vector<float>> data_vector_;

  struct RasterizeContext {
    SkCanvas *canvas;
    SkPaint paint;
    bool invertColors = false;
    sk_sp<SkColorFilter> colorFilter;
  };

  static sk_sp<SkColorFilter> makeColorFilter(RasterizeContext& context);

  FOR_EACH_CANVAS_OP(CANVAS_OP_DECLARE_OP)

  bool HasOp() { return ops_it_ < ops_end_; }
  CanvasOp GetOp() { return static_cast<CanvasOp>(*ops_it_++); }

  std::string DescribeNextOp() {
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

  SkScalar GetScalar() { return static_cast<SkScalar>(*data_it_++); }
  uint32_t GetUint32() { union { float f; uint32_t i; } data; data.f = *data_it_++; return data.i; }

  SkScalar GetAngle() { return GetScalar() * 180.0 / M_PI; }
  SkBlendMode GetBlendMode() { return static_cast<SkBlendMode>(GetUint32()); }
  SkPoint GetPoint() { return SkPoint::Make(GetScalar(), GetScalar()); }
  SkColor GetColor() { return static_cast<SkColor>(GetUint32()); }
  SkRect GetRect() { return SkRect::MakeLTRB(GetScalar(), GetScalar(), GetScalar(), GetScalar()); }
  SkRRect GetRoundRect() {
    SkRect rect = GetRect();
    SkVector radii[4] = {
      // SkRRect Radii order is UL, UR, LR, LL as per SkRRect::Corner indices
      { GetScalar(), GetScalar() },
      { GetScalar(), GetScalar() },
      { GetScalar(), GetScalar() },
      { GetScalar(), GetScalar() },
    };

    SkRRect rrect;
    rrect.setRectRadii(rect, radii);
    return rrect;
  }
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_INTERPRETER_H_
