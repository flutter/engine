// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_INTERPRETER_H_
#define FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_INTERPRETER_H_

#include <sstream>

#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkVertices.h"

namespace flutter {

enum CanvasOpArg {
  empty,  // Forces all following enum values to be non-zero
  scalar,
  color,
  blend_mode,
  angle,
  point,
  rect,
  round_rect,
  uint32_list,
  scalar_list,
  matrix_row3,
  image,
  path,
  vertices,
  skpicture,
  display_list,
  shader,
  color_filter,
  image_filter,
};
#define CANVAS_OP_ARG_SHIFT 5
#define CANVAS_OP_ARG_MASK  ((1 << CANVAS_OP_ARG_SHIFT) - 1)

#define CANVAS_OP_APPEND_ARG(arg_list, arg) \
  (((arg_list) << CANVAS_OP_ARG_SHIFT) | CanvasOpArg::arg)
#define CANVAS_OP_ARGS1(arg) \
  (CanvasOpArg::arg)
#define CANVAS_OP_ARGS2(arg1, arg2) \
  CANVAS_OP_APPEND_ARG(CANVAS_OP_ARGS1(arg2), arg1)
#define CANVAS_OP_ARGS3(arg1, arg2, arg3) \
  CANVAS_OP_APPEND_ARG(CANVAS_OP_ARGS2(arg2, arg3), arg1)
#define CANVAS_OP_ARGS4(arg1, arg2, arg3, arg4) \
  CANVAS_OP_APPEND_ARG(CANVAS_OP_ARGS3(arg2, arg3, arg4), arg1)
#define CANVAS_OP_ARGS5(arg1, arg2, arg3, arg4, arg5) \
  CANVAS_OP_APPEND_ARG(CANVAS_OP_ARGS4(arg2, arg3, arg4, arg5), arg1)
#define CANVAS_OP_ARGS6(arg1, arg2, arg3, arg4, arg5, arg6) \
  CANVAS_OP_APPEND_ARG(CANVAS_OP_ARGS5(arg2, arg3, arg4, arg5, arg6), arg1)

#define CANVAS_OP_ARGS__                    0
#define CANVAS_OP_ARGS_Scalar               CANVAS_OP_ARGS1(scalar)
#define CANVAS_OP_ARGS_2Scalars             CANVAS_OP_ARGS2(scalar, scalar)
#define CANVAS_OP_ARGS_Angle                CANVAS_OP_ARGS1(angle)
#define CANVAS_OP_ARGS_Color                CANVAS_OP_ARGS1(color)
#define CANVAS_OP_ARGS_BlendMode            CANVAS_OP_ARGS1(blend_mode)
#define CANVAS_OP_ARGS_Color_BlendMode      CANVAS_OP_ARGS2(color, blend_mode)
#define CANVAS_OP_ARGS_Point_Scalar         CANVAS_OP_ARGS2(point, scalar)
#define CANVAS_OP_ARGS_2Points              CANVAS_OP_ARGS2(point, point)
#define CANVAS_OP_ARGS_Rect                 CANVAS_OP_ARGS1(rect)
#define CANVAS_OP_ARGS_Rect_2Angles         CANVAS_OP_ARGS3(rect, angle, angle)
#define CANVAS_OP_ARGS_RoundRect            CANVAS_OP_ARGS1(round_rect)
#define CANVAS_OP_ARGS_2RoundRects          CANVAS_OP_ARGS2(round_rect, round_rect)
#define CANVAS_OP_ARGS_ScalarList           CANVAS_OP_ARGS1(scalar_list)
#define CANVAS_OP_ARGS_Matrix2x3            CANVAS_OP_ARGS2(matrix_row3, matrix_row3)
#define CANVAS_OP_ARGS_Matrix3x3            CANVAS_OP_ARGS3(matrix_row3, matrix_row3, matrix_row3)
#define CANVAS_OP_ARGS_Path                 CANVAS_OP_ARGS1(path)
#define CANVAS_OP_ARGS_Path_Scalar_Scalar   CANVAS_OP_ARGS3(path, scalar, scalar)
#define CANVAS_OP_ARGS_Vertices_BlendMode   CANVAS_OP_ARGS2(vertices, blend_mode)
#define CANVAS_OP_ARGS_Image_Point          CANVAS_OP_ARGS2(image, point)
#define CANVAS_OP_ARGS_Image_2Rects         CANVAS_OP_ARGS3(image, rect, rect)
#define CANVAS_OP_ARGS_Atlas                CANVAS_OP_ARGS4(image, blend_mode, scalar_list, scalar_list)
#define CANVAS_OP_ARGS_Atlas_Colors         CANVAS_OP_ARGS5(image, blend_mode, scalar_list, scalar_list, uint32_list)
#define CANVAS_OP_ARGS_Atlas_Rect           CANVAS_OP_ARGS5(image, blend_mode, scalar_list, scalar_list, rect)
#define CANVAS_OP_ARGS_Atlas_Colors_Rect    CANVAS_OP_ARGS6(image, blend_mode, scalar_list, scalar_list, uint32_list, rect)
#define CANVAS_OP_ARGS_SkPicture            CANVAS_OP_ARGS1(skpicture)
#define CANVAS_OP_ARGS_DisplayList          CANVAS_OP_ARGS1(display_list)
#define CANVAS_OP_ARGS_Shader               CANVAS_OP_ARGS1(shader)
#define CANVAS_OP_ARGS_ColorFilter          CANVAS_OP_ARGS1(color_filter)
#define CANVAS_OP_ARGS_ImageFilter          CANVAS_OP_ARGS1(image_filter)

// opName (matches Dart enum name), argListDescriptor
#define FOR_EACH_CANVAS_OP(V)                       \
  V(setAA,                      _)                  \
  V(clearAA,                    _)                  \
  V(setDither,                  _)                  \
  V(clearDither,                _)                  \
  V(setInvertColors,            _)                  \
  V(clearInvertColors,          _)                  \
  V(setFillStyle,               _)                  \
  V(setStrokeStyle,             _)                  \
                                                    \
  V(setCapsButt,                _)                  \
  V(setCapsRound,               _)                  \
  V(setCapsSquare,              _)                  \
  V(setJoinsBevel,              _)                  \
  V(setJoinsMiter,              _)                  \
  V(setJoinsRound,              _)                  \
                                                    \
  V(setStrokeWidth,             Scalar)             \
  V(setMiterLimit,              Scalar)             \
                                                    \
  V(setFilterQualityNearest,    _)                  \
  V(setFilterQualityLinear,     _)                  \
  V(setFilterQualityMipmap,     _)                  \
  V(setFilterQualityCubic,      _)                  \
                                                    \
  V(setColor,                   Color)              \
  V(setBlendMode,               BlendMode)          \
                                                    \
  V(setShader,                  Shader)             \
  V(clearShader,                _)                  \
  V(setColorFilter,             ColorFilter)        \
  V(clearColorFilter,           _)                  \
  V(setImageFilter,             ImageFilter)        \
  V(clearImageFilter,           _)                  \
                                                    \
  V(clearMaskFilter,            _)                  \
  V(setMaskFilterNormal,        Scalar)             \
  V(setMaskFilterSolid,         Scalar)             \
  V(setMaskFilterOuter,         Scalar)             \
  V(setMaskFilterInner,         Scalar)             \
                                                    \
  V(save,                       _)                  \
  V(saveLayer,                  _)                  \
  V(saveLayerBounds,            Rect)               \
  V(restore,                    _)                  \
                                                    \
  V(translate,                  2Scalars)           \
  V(scale,                      2Scalars)           \
  V(rotate,                     Angle)              \
  V(skew,                       2Scalars)           \
  V(transform2x3,               Matrix2x3)          \
  V(transform3x3,               Matrix3x3)          \
                                                    \
  V(clipRect,                   Rect)               \
  V(clipRectAA,                 Rect)               \
  V(clipRectDiff,               Rect)               \
  V(clipRectAADiff,             Rect)               \
  V(clipRRect,                  RoundRect)          \
  V(clipRRectAA,                RoundRect)          \
  V(clipPath,                   Path)               \
  V(clipPathAA,                 Path)               \
                                                    \
  V(drawPaint,                  _)                  \
  V(drawColor,                  Color_BlendMode)    \
                                                    \
  V(drawLine,                   2Points)            \
  V(drawRect,                   Rect)               \
  V(drawOval,                   Rect)               \
  V(drawCircle,                 Point_Scalar)       \
  V(drawRRect,                  RoundRect)          \
  V(drawDRRect,                 2RoundRects)        \
  V(drawArc,                    Rect_2Angles)       \
  V(drawArcCenter,              Rect_2Angles)       \
  V(drawPath,                   Path)               \
                                                    \
  V(drawPoints,                 ScalarList)         \
  V(drawLines,                  ScalarList)         \
  V(drawPolygon,                ScalarList)         \
  V(drawVertices,               Vertices_BlendMode) \
                                                    \
  V(drawImage,                  Image_Point)        \
  V(drawImageRect,              Image_2Rects)       \
  V(drawImageNine,              Image_2Rects)       \
  V(drawAtlas,                  Atlas)              \
  V(drawAtlasColored,           Atlas_Colors)       \
  V(drawAtlasCulled,            Atlas_Rect)         \
  V(drawAtlasColoredCulled,     Atlas_Colors_Rect)  \
                                                    \
  V(drawSkPicture,              SkPicture)          \
  V(drawDisplayList,            DisplayList)        \
  V(drawShadow,                 Path_Scalar_Scalar) \
  V(drawShadowOccluded,         Path_Scalar_Scalar)

#define CANVAS_OP_MAKE_ENUM(name, args) cops_##name,
enum CanvasOp {
  FOR_EACH_CANVAS_OP(CANVAS_OP_MAKE_ENUM)
};

class DisplayListRefHolder;

struct DisplayListData {
  std::shared_ptr<std::vector<uint8_t>> ops_vector;
  std::shared_ptr<std::vector<float>> data_vector;
  std::shared_ptr<std::vector<DisplayListRefHolder>> ref_vector;
};

class DisplayListRefHolder {
 public:
  sk_sp<SkColorFilter> colorFilter;
  sk_sp<SkImageFilter> imageFilter;
  sk_sp<SkImage> image;
  sk_sp<SkVertices> vertices;
  sk_sp<SkShader> shader;
  sk_sp<SkPicture> picture;
  sk_sp<SkData> pathData;
  DisplayListData displayList;
};

class DisplayListInterpreter {
 public:
  DisplayListInterpreter(DisplayListData data);

  DisplayListInterpreter(std::shared_ptr<std::vector<uint8_t>> ops,
                         std::shared_ptr<std::vector<float>> data,
                         std::shared_ptr<std::vector<DisplayListRefHolder>> refs);

  void Rasterize(SkCanvas *canvas);

  void Describe();

  static const std::vector<std::string> opNames;
  static const std::vector<uint32_t> opArguments;

  static const SkSamplingOptions NearestSampling;
  static const SkSamplingOptions LinearSampling;
  static const SkSamplingOptions MipmapSampling;
  static const SkSamplingOptions CubicSampling;

 private:
  std::shared_ptr<std::vector<uint8_t>> ops_vector_;
  std::shared_ptr<std::vector<float>> data_vector_;
  std::shared_ptr<std::vector<DisplayListRefHolder>> ref_vector_;

  class Iterator {
   public:
    Iterator(const DisplayListInterpreter* interpreter)
      : ops(interpreter->ops_vector_->begin()),
        ops_end(interpreter->ops_vector_->end()),
        data(interpreter->data_vector_->begin()),
        data_end(interpreter->data_vector_->end()),
        refs(interpreter->ref_vector_->begin()),
        refs_end(interpreter->ref_vector_->end()) {}

    Iterator(const Iterator& iterator)
      : ops(iterator.ops),
        ops_end(iterator.ops_end),
        data(iterator.data),
        data_end(iterator.data_end),
        refs(iterator.refs),
        refs_end(iterator.refs_end) {}

    bool HasOp() { return ops < ops_end; }
    CanvasOp GetOp() { return static_cast<CanvasOp>(*ops++); }

    void skipData(int n) { data += n; }
    SkScalar GetScalar() { return static_cast<SkScalar>(*data++); }
    uint32_t GetUint32() { union { float f; uint32_t i; } u; u.f = *data++; return u.i; }

    SkScalar GetAngle() { return GetScalar() * 180.0 / M_PI; }
    SkBlendMode GetBlendMode() { return static_cast<SkBlendMode>(GetUint32()); }
    SkColor GetColor() { return static_cast<SkColor>(GetUint32()); }

    SkPoint GetPoint() { return SkPoint::Make(GetScalar(), GetScalar()); }
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

    uint32_t GetIntList(uint32_t **int_ptr) {
      uint32_t len = GetUint32();
      *int_ptr = (uint32_t *) &*data;
      skipData(len);
      return len;
    }

    uint32_t GetFloatList(SkScalar **flt_ptr) {
      uint32_t len = GetUint32();
      *flt_ptr = &*data;
      skipData(len);
      return len;
    }

    void skipSkRef() { refs++; }
    const sk_sp<SkColorFilter> GetColorFilter() { return (refs++)->colorFilter; }
    const sk_sp<SkImageFilter> GetImageFilter() { return (refs++)->imageFilter; }
    const sk_sp<SkImage> GetImage() { return (refs++)->image; }
    const sk_sp<SkVertices> GetVertices() { return (refs++)->vertices; }
    const sk_sp<SkShader> GetShader() { return (refs++)->shader; }
    const sk_sp<SkPicture> GetSkPicture() { return (refs++)->picture; }
    const DisplayListData GetDisplayList() { return (refs++)->displayList; }
    void GetPath(SkPath& path) {
      SkData* data = (refs++)->pathData.get();
      path.readFromMemory(data->data(), data->size());
    }

    std::vector<uint8_t>::iterator ops;
    const std::vector<uint8_t>::iterator ops_end;
    std::vector<float>::iterator data;
    const std::vector<float>::iterator data_end;
    std::vector<DisplayListRefHolder>::iterator refs;
    const std::vector<DisplayListRefHolder>::iterator refs_end;
  };

  struct RasterizeContext {
    SkCanvas *canvas;
    SkPaint paint;
    bool invertColors = false;
    sk_sp<SkColorFilter> colorFilter;
    SkFilterMode filterMode;
    SkSamplingOptions sampling;
  };

  static sk_sp<SkColorFilter> makeColorFilter(RasterizeContext& context);

#define CANVAS_OP_DECLARE_OP(name, args) \
  void execute_##name(RasterizeContext& context, Iterator& it);

  FOR_EACH_CANVAS_OP(CANVAS_OP_DECLARE_OP)

  std::string DescribeNextOp(const Iterator& it);
  std::string DescribeOneOp(Iterator& it);
};

}  // namespace flutter

#endif  // FLUTTER_LIB_UI_PAINTING_DISPLAY_LIST_INTERPRETER_H_
