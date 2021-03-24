// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_DISPLAY_LIST_H_
#define FLUTTER_FLOW_DISPLAY_LIST_H_

#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkRRect.h"

// namespace tonic {
// class DartLibraryNatives;
// }  // namespace tonic

namespace flutter {

enum CanvasOp {
  cops_setAA,
  cops_clearAA,
  cops_setInvertColors,
  cops_clearInvertColors,
  cops_setFillStyle,
  cops_setStrokeStyle,

  cops_setCapsButt,
  cops_setCapsRound,
  cops_setCapsSquare,
  cops_setJoinsBevel,
  cops_setJoinsMiter,
  cops_setJoinsRound,

  cops_setStrokeWidth,
  cops_setMiterLimit,

  cops_setFilterQualityNearest,
  cops_setFilterQualityLinear,
  cops_setFilterQualityMipmap,
  cops_setFilterQualityCubic,

  cops_setColor,
  cops_setBlendMode,

  cops_setShader,
  cops_clearShader,
  cops_setColorFilter,
  cops_clearColorFilter,
  cops_setImageFilter,
  cops_clearImageFilter,

  cops_clearMaskFilter,
  cops_setMaskFilterNormal,
  cops_setMaskFilterSolid,
  cops_setMaskFilterOuter,
  cops_setMaskFilterInner,

  cops_save,
  cops_saveLayer,
  cops_saveLayerBounds,
  cops_restore,

  cops_translate,
  cops_scale,
  cops_rotate,
  cops_skew,
  cops_transform,

  cops_clipRect,
  cops_clipRectAA,
  cops_clipRectDiff,
  cops_clipRectAADiff,
  cops_clipRRect,
  cops_clipRRectAA,
  cops_clipPath,
  cops_clipPathAA,

  cops_drawPaint,
  cops_drawColor,

  cops_drawLine,
  cops_drawRect,
  cops_drawOval,
  cops_drawCircle,
  cops_drawRRect,
  cops_drawDRRect,
  cops_drawArc,
  cops_drawArcCenter,
  cops_drawPath,

  cops_drawPoints,
  cops_drawLines,
  cops_drawPolygon,

  cops_drawImage,
  cops_drawImageRect,
  cops_drawImageNine,
  cops_drawAtlas,
  cops_drawAtlasColored,
  cops_drawAtlasCulled,
  cops_drawAtlasColoredCulled,

  cops_drawParagraph,
  cops_drawPicture,
  cops_drawShadow,
  cops_drawShadowOccluded,
};

class DisplayListRasterizer {
 public:
  DisplayListRasterizer(std::vector<uint8_t> ops, std::vector<uint32_t> data);

  void Rasterize(SkCanvas *canvas);

 private:
  std::vector<uint8_t>::iterator ops_it_;
  std::vector<uint8_t>::iterator ops_end_;
  std::vector<uint32_t>::iterator data_it_;

  SkScalar GetScalar() { return static_cast<SkScalar>(*data_it_++); }
  SkBlendMode GetBlendMode() { return static_cast<SkBlendMode>(*data_it_++); }
  SkPoint GetPoint() { return SkPoint::Make(GetScalar(), GetScalar()); }
  SkColor GetColor() { return *data_it_++; }
  SkRect GetRect() { return SkRect::MakeLTRB(GetScalar(), GetScalar(), GetScalar(), GetScalar()); }
  SkRRect GetRoundRect() {
    SkRect rect = GetRect();
    SkVector radii[4] = {
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

// class DisplayList : public RefCountedDartWrappable<DisplayList> {
//   DEFINE_WRAPPERTYPEINFO();
//   FML_FRIEND_MAKE_REF_COUNTED(DisplayList);

//  public:

//   ~DisplayList() override;
//   static fml::RefPtr<DisplayList> Create(Dart_Handle dart_handle,
//                                      flutter::SkiaGPUObject<SkPicture> picture);

//   Dart_Handle toImage(uint32_t width,
//                       uint32_t height,
//                       Dart_Handle raw_image_callback);

//   void dispose();

//   size_t GetAllocationSize() const override;

//   static void RegisterNatives(tonic::DartLibraryNatives* natives);

//   static Dart_Handle RasterizeToImage(sk_sp<SkPicture> picture,
//                                       uint32_t width,
//                                       uint32_t height,
//                                       Dart_Handle raw_image_callback);

//  private:
//   Picture(flutter::SkiaGPUObject<SkPicture> picture);
// };

}  // namespace flutter

#endif  // FLUTTER_FLOW_DISPLAY_LIST_H_
