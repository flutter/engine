// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/display_list_canvas.h"

#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkRSXform.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkVertices.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/effects/SkImageFilters.h"

#include <cmath>

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

constexpr SkPoint end_points[] = {
    {0, 0},
    {100, 100},
};
constexpr SkColor colors[] = {
    SK_ColorGREEN,
    SK_ColorYELLOW,
    SK_ColorBLUE,
};
constexpr float stops[] = {
    0.0,
    0.5,
    1.0,
};
constexpr float rotate_color_matrix[20] = {
    0, 1, 0, 0, 0,  //
    0, 0, 1, 0, 0,  //
    1, 0, 0, 0, 0,  //
    0, 0, 0, 1, 0,  //
};

constexpr SkPoint TestPoints[] = {
    {10, 10},
    {20, 20},
    {10, 20},
    {20, 10},
};
#define TestPointCount sizeof(TestPoints) / (sizeof(TestPoints[0]))

static const sk_sp<SkShader> TestShader1 =
    SkGradientShader::MakeLinear(end_points,
                                 colors,
                                 stops,
                                 3,
                                 SkTileMode::kMirror,
                                 0,
                                 nullptr);
// TestShader2 is identical to TestShader1 and points out that we cannot
// perform a deep compare over our various sk_sp objects because the
// DisplayLists constructed with the two do not compare == below.
static const sk_sp<SkShader> TestShader2 =
    SkGradientShader::MakeLinear(end_points,
                                 colors,
                                 stops,
                                 3,
                                 SkTileMode::kMirror,
                                 0,
                                 nullptr);
static const sk_sp<SkShader> TestShader3 =
    SkGradientShader::MakeLinear(end_points,
                                 colors,
                                 stops,
                                 3,
                                 SkTileMode::kDecal,
                                 0,
                                 nullptr);
static const sk_sp<SkImageFilter> TestImageFilter =
    SkImageFilters::Blur(5.0, 5.0, SkTileMode::kDecal, nullptr, nullptr);
static const sk_sp<SkColorFilter> TestColorFilter =
    SkColorFilters::Matrix(rotate_color_matrix);
static const sk_sp<SkMaskFilter> TestMaskFilter =
    SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5.0);
constexpr SkRect TestBounds = SkRect::MakeLTRB(10, 10, 50, 60);
static const SkRRect TestRRect = SkRRect::MakeRectXY(TestBounds, 5, 5);
static const SkRRect TestInnerRRect =
    SkRRect::MakeRectXY(TestBounds.makeInset(5, 5), 2, 2);
static const SkPath TestPath1 = SkPath::Rect(TestBounds);
static const SkPath TestPath2 = SkPath::Oval(TestBounds);

static sk_sp<SkImage> MakeTestImage(int w, int h, int checker_size) {
  sk_sp<SkSurface> surface = SkSurface::MakeRasterN32Premul(w, h);
  SkCanvas* canvas = surface->getCanvas();
  SkPaint p0, p1;
  p0.setStyle(SkPaint::kFill_Style);
  p0.setColor(SK_ColorGREEN);
  p1.setStyle(SkPaint::kFill_Style);
  p1.setColor(SK_ColorBLUE);
  p1.setAlpha(128);
  for (int y = 0; y < w; y += checker_size) {
    for (int x = 0; x < h; x += checker_size) {
      SkPaint& cellp = ((x + y) & 1) == 0 ? p0 : p1;
      canvas->drawRect(SkRect::MakeXYWH(x, y, checker_size, checker_size),
                       cellp);
    }
  }
  return surface->makeImageSnapshot();
}
static sk_sp<SkImage> TestImage1 = MakeTestImage(40, 40, 5);
static sk_sp<SkImage> TestImage2 = MakeTestImage(20, 20, 5);

static sk_sp<SkVertices> TestVertices1 =
    SkVertices::MakeCopy(SkVertices::kTriangles_VertexMode,
                         3,
                         TestPoints,
                         nullptr,
                         colors);
static sk_sp<SkVertices> TestVertices2 =
    SkVertices::MakeCopy(SkVertices::kTriangleFan_VertexMode,
                         3,
                         TestPoints,
                         nullptr,
                         colors);

static sk_sp<SkPicture> MakeTestPicture(int w, int h, SkColor color) {
  SkPictureRecorder recorder;
  SkCanvas* cv = recorder.beginRecording(TestBounds);
  SkPaint paint;
  paint.setColor(color);
  paint.setStyle(SkPaint::kFill_Style);
  cv->drawRect(SkRect::MakeWH(w, h), paint);
  return recorder.finishRecordingAsPicture();
}
static sk_sp<SkPicture> TestPicture1 = MakeTestPicture(20, 20, SK_ColorGREEN);
static sk_sp<SkPicture> TestPicture2 = MakeTestPicture(25, 25, SK_ColorBLUE);

static sk_sp<DisplayList> MakeTestDisplayList(int w, int h, SkColor color) {
  DisplayListBuilder builder;
  builder.setColor(color);
  builder.drawRect(SkRect::MakeWH(w, h));
  return builder.build();
}
static sk_sp<DisplayList> TestDisplayList1 =
    MakeTestDisplayList(20, 20, SK_ColorGREEN);
static sk_sp<DisplayList> TestDisplayList2 =
    MakeTestDisplayList(25, 25, SK_ColorBLUE);

typedef const std::function<void(DisplayListBuilder&)> DlInvoker;

struct DisplayListInvocation {
  int opCount;
  size_t byteCount;
  DlInvoker invoker;
  sk_sp<DisplayList> build() {
    DisplayListBuilder builder;
    invoker(builder);
    return builder.build();
  }
};

struct DisplayListInvocationGroup {
  std::string op_name;
  std::vector<DisplayListInvocation> variants;
};

std::vector<DisplayListInvocationGroup> allGroups = {
  { "SetAA", {
      {1, 8, [](DisplayListBuilder& b) {b.setAA(false);}},
      {1, 8, [](DisplayListBuilder& b) {b.setAA(true);}},
    }
  },
  { "SetDither", {
      {1, 8, [](DisplayListBuilder& b) {b.setDither(false);}},
      {1, 8, [](DisplayListBuilder& b) {b.setDither(true);}},
    }
  },
  { "SetInvertColors", {
      {1, 8, [](DisplayListBuilder& b) {b.setInvertColors(false);}},
      {1, 8, [](DisplayListBuilder& b) {b.setInvertColors(true);}},
    }
  },
  { "SetStrokeCap", {
      {1, 8, [](DisplayListBuilder& b) {b.setCaps(SkPaint::kButt_Cap);}},
      {1, 8, [](DisplayListBuilder& b) {b.setCaps(SkPaint::kRound_Cap);}},
      {1, 8, [](DisplayListBuilder& b) {b.setCaps(SkPaint::kSquare_Cap);}},
    }
  },
  { "SetStrokeJoin", {
      {1, 8, [](DisplayListBuilder& b) {b.setJoins(SkPaint::kBevel_Join);}},
      {1, 8, [](DisplayListBuilder& b) {b.setJoins(SkPaint::kRound_Join);}},
      {1, 8, [](DisplayListBuilder& b) {b.setJoins(SkPaint::kMiter_Join);}},
    }
  },
  { "SetDrawStyle", {
      {1, 8, [](DisplayListBuilder& b) {b.setDrawStyle(SkPaint::kFill_Style);}},
      {1, 8, [](DisplayListBuilder& b) {b.setDrawStyle(SkPaint::kStroke_Style);}},
    }
  },
  { "SetStrokeWidth", {
      {1, 8, [](DisplayListBuilder& b) {b.setStrokeWidth(0.0);}},
      {1, 8, [](DisplayListBuilder& b) {b.setStrokeWidth(5.0);}},
    }
  },
  { "SetMiterLimit", {
      {1, 8, [](DisplayListBuilder& b) {b.setMiterLimit(0.0);}},
      {1, 8, [](DisplayListBuilder& b) {b.setMiterLimit(5.0);}},
    }
  },
  { "SetColor", {
      {1, 8, [](DisplayListBuilder& b) {b.setColor(SK_ColorGREEN);}},
      {1, 8, [](DisplayListBuilder& b) {b.setColor(SK_ColorBLUE);}},
    }
  },
  { "SetBlendMode", {
      {1, 8, [](DisplayListBuilder& b) {b.setBlendMode(SkBlendMode::kSrcIn);}},
      {1, 8, [](DisplayListBuilder& b) {b.setBlendMode(SkBlendMode::kDstIn);}},
    }
  },
  { "SetFilterQuality", {
      {1, 8, [](DisplayListBuilder& b) {b.setFilterQuality(kNone_SkFilterQuality);}},
      {1, 8, [](DisplayListBuilder& b) {b.setFilterQuality(kLow_SkFilterQuality);}},
      {1, 8, [](DisplayListBuilder& b) {b.setFilterQuality(kMedium_SkFilterQuality);}},
      {1, 8, [](DisplayListBuilder& b) {b.setFilterQuality(kHigh_SkFilterQuality);}},
    }
  },
  { "SetShader", {
      {1, 8, [](DisplayListBuilder& b) {b.setShader(nullptr);}},
      {1, 16, [](DisplayListBuilder& b) {b.setShader(TestShader1);}},
      {1, 16, [](DisplayListBuilder& b) {b.setShader(TestShader2);}},
      {1, 16, [](DisplayListBuilder& b) {b.setShader(TestShader3);}},
    }
  },
  { "SetImageFilter", {
      {1, 8, [](DisplayListBuilder& b) {b.setImageFilter(nullptr);}},
      {1, 16, [](DisplayListBuilder& b) {b.setImageFilter(TestImageFilter);}},
    }
  },
  { "SetColorFilter", {
      {1, 8, [](DisplayListBuilder& b) {b.setColorFilter(nullptr);}},
      {1, 16, [](DisplayListBuilder& b) {b.setColorFilter(TestColorFilter);}},
    }
  },
  { "SetMaskFilter", {
      {1, 16, [](DisplayListBuilder& b) {b.setMaskFilter(nullptr);}},
      {1, 16, [](DisplayListBuilder& b) {b.setMaskFilter(TestMaskFilter);}},
      {1, 8, [](DisplayListBuilder& b) {b.setMaskBlurFilter(kNormal_SkBlurStyle, 3.0);}},
      {1, 8, [](DisplayListBuilder& b) {b.setMaskBlurFilter(kNormal_SkBlurStyle, 5.0);}},
      {1, 8, [](DisplayListBuilder& b) {b.setMaskBlurFilter(kSolid_SkBlurStyle, 3.0);}},
      {1, 8, [](DisplayListBuilder& b) {b.setMaskBlurFilter(kInner_SkBlurStyle, 3.0);}},
      {1, 8, [](DisplayListBuilder& b) {b.setMaskBlurFilter(kOuter_SkBlurStyle, 3.0);}},
    }
  },
  { "Save(Layer)+Restore", {
      {2, 16, [](DisplayListBuilder& b) {b.save(); b.restore();}},
      {2, 16, [](DisplayListBuilder& b) {b.saveLayer(nullptr, false); b.restore(); }},
      {2, 16, [](DisplayListBuilder& b) {b.saveLayer(nullptr, true); b.restore(); }},
      {2, 32, [](DisplayListBuilder& b) {b.saveLayer(&TestBounds, false); b.restore(); }},
      {2, 32, [](DisplayListBuilder& b) {b.saveLayer(&TestBounds, true); b.restore(); }},
    }
  },
  { "Translate", {
      {1, 16, [](DisplayListBuilder& b) {b.translate(0, 0);}},
      {1, 16, [](DisplayListBuilder& b) {b.translate(5, 15);}},
    }
  },
  { "Scale", {
      {1, 16, [](DisplayListBuilder& b) {b.scale(1, 1);}},
      {1, 16, [](DisplayListBuilder& b) {b.scale(2, 3);}},
    }
  },
  { "Rotate", {
      {1, 8, [](DisplayListBuilder& b) {b.rotate(0);}},
      {1, 8, [](DisplayListBuilder& b) {b.rotate(45);}},
    }
  },
  { "Skew", {
      {1, 16, [](DisplayListBuilder& b) {b.skew(0, 0);}},
      {1, 16, [](DisplayListBuilder& b) {b.skew(0.1, 0.2);}},
    }
  },
  { "Transform2x3", {
      {1, 32, [](DisplayListBuilder& b) {b.transform2x3(1, 0, 0, 0, 1, 0);}},
      {1, 32, [](DisplayListBuilder& b) {b.transform2x3(0, 1, 12, 1, 0, 33);}},
    }
  },
  { "Transform3x3", {
      {1, 40, [](DisplayListBuilder& b) {b.transform3x3(1, 0, 0, 0, 1, 0, 0, 0, 1);}},
      {1, 40, [](DisplayListBuilder& b) {b.transform3x3(0, 1, 12, 1, 0, 33, 0, 0, 12);}},
    }
  },
  { "ClipRect", {
      {1, 24, [](DisplayListBuilder& b) {b.clipRect(TestBounds, true, SkClipOp::kIntersect);}},
      {1, 24, [](DisplayListBuilder& b) {b.clipRect(TestBounds.makeOffset(1, 1),
                                                    true, SkClipOp::kIntersect);}},
      {1, 24, [](DisplayListBuilder& b) {b.clipRect(TestBounds, false, SkClipOp::kIntersect);}},
      {1, 24, [](DisplayListBuilder& b) {b.clipRect(TestBounds, true, SkClipOp::kDifference);}},
      {1, 24, [](DisplayListBuilder& b) {b.clipRect(TestBounds, false, SkClipOp::kDifference);}},
    }
  },
  { "ClipRRect", {
      {1, 64, [](DisplayListBuilder& b) {b.clipRRect(TestRRect, true, SkClipOp::kIntersect);}},
      {1, 64, [](DisplayListBuilder& b) {b.clipRRect(TestRRect.makeOffset(1, 1),
                                                     true, SkClipOp::kIntersect);}},
      {1, 64, [](DisplayListBuilder& b) {b.clipRRect(TestRRect, false, SkClipOp::kIntersect);}},
      {1, 64, [](DisplayListBuilder& b) {b.clipRRect(TestRRect, true, SkClipOp::kDifference);}},
      {1, 64, [](DisplayListBuilder& b) {b.clipRRect(TestRRect, false, SkClipOp::kDifference);}},
    }
  },
  { "ClipPath", {
      {1, 24, [](DisplayListBuilder& b) {b.clipPath(TestPath1, true, SkClipOp::kIntersect);}},
      {1, 24, [](DisplayListBuilder& b) {b.clipPath(TestPath2, true, SkClipOp::kIntersect);}},
      {1, 24, [](DisplayListBuilder& b) {b.clipPath(TestPath1, false, SkClipOp::kIntersect);}},
      {1, 24, [](DisplayListBuilder& b) {b.clipPath(TestPath1, true, SkClipOp::kDifference);}},
      {1, 24, [](DisplayListBuilder& b) {b.clipPath(TestPath1, false, SkClipOp::kDifference);}},
    }
  },
  { "DrawPaint", {
      {1, 8, [](DisplayListBuilder& b) {b.drawPaint();}},
    }
  },
  { "DrawColor", {
      {1, 16, [](DisplayListBuilder& b) {b.drawColor(SK_ColorBLUE, SkBlendMode::kSrcIn);}},
      {1, 16, [](DisplayListBuilder& b) {b.drawColor(SK_ColorBLUE, SkBlendMode::kDstIn);}},
      {1, 16, [](DisplayListBuilder& b) {b.drawColor(SK_ColorCYAN, SkBlendMode::kSrcIn);}},
    }
  },
  { "DrawLine", {
      {1, 24, [](DisplayListBuilder& b) {b.drawLine({0, 0}, {10, 10});}},
      {1, 24, [](DisplayListBuilder& b) {b.drawLine({0, 1}, {10, 10});}},
      {1, 24, [](DisplayListBuilder& b) {b.drawLine({0, 0}, {20, 10});}},
      {1, 24, [](DisplayListBuilder& b) {b.drawLine({0, 0}, {10, 20});}},
    }
  },
  { "DrawRect", {
      {1, 24, [](DisplayListBuilder& b) {b.drawRect({0, 0, 10, 10});}},
      {1, 24, [](DisplayListBuilder& b) {b.drawRect({0, 1, 10, 10});}},
      {1, 24, [](DisplayListBuilder& b) {b.drawRect({0, 0, 20, 10});}},
      {1, 24, [](DisplayListBuilder& b) {b.drawRect({0, 0, 10, 20});}},
    }
  },
  { "DrawOval", {
      {1, 24, [](DisplayListBuilder& b) {b.drawOval({0, 0, 10, 10});}},
      {1, 24, [](DisplayListBuilder& b) {b.drawOval({0, 1, 10, 10});}},
      {1, 24, [](DisplayListBuilder& b) {b.drawOval({0, 0, 20, 10});}},
      {1, 24, [](DisplayListBuilder& b) {b.drawOval({0, 0, 10, 20});}},
    }
  },
  { "DrawCircle", {
      {1, 16, [](DisplayListBuilder& b) {b.drawCircle({0, 0}, 10);}},
      {1, 16, [](DisplayListBuilder& b) {b.drawCircle({0, 5}, 10);}},
      {1, 16, [](DisplayListBuilder& b) {b.drawCircle({0, 0}, 20);}},
    }
  },
  { "DrawRRect", {
      {1, 56, [](DisplayListBuilder& b) {b.drawRRect(TestRRect);}},
      {1, 56, [](DisplayListBuilder& b) {b.drawRRect(TestRRect.makeOffset(5, 5));}},
    }
  },
  { "DrawDRRect", {
      {1, 112, [](DisplayListBuilder& b) {b.drawDRRect(TestRRect, TestInnerRRect);}},
      {1, 112, [](DisplayListBuilder& b) {b.drawDRRect(TestRRect.makeOffset(5, 5),
                                                       TestInnerRRect.makeOffset(4, 4));}},
    }
  },
  { "DrawPath", {
      {1, 24, [](DisplayListBuilder& b) {b.drawPath(TestPath1);}},
      {1, 24, [](DisplayListBuilder& b) {b.drawPath(TestPath2);}},
    }
  },
  { "DrawArc", {
      {1, 32, [](DisplayListBuilder& b) {b.drawArc(TestBounds, 45, 270, false);}},
      {1, 32, [](DisplayListBuilder& b) {b.drawArc(TestBounds.makeOffset(1, 1),
                                                   45, 270, false);}},
      {1, 32, [](DisplayListBuilder& b) {b.drawArc(TestBounds, 30, 270, false);}},
      {1, 32, [](DisplayListBuilder& b) {b.drawArc(TestBounds, 45, 260, false);}},
      {1, 32, [](DisplayListBuilder& b) {b.drawArc(TestBounds, 45, 270, true);}},
    }
  },
  { "DrawPoints", {
      {1, 8 + TestPointCount * 8,
       [](DisplayListBuilder& b) {b.drawPoints(SkCanvas::kPoints_PointMode,
                                               TestPointCount,
                                               TestPoints);}},
      {1, 8 + (TestPointCount - 1) * 8,
       [](DisplayListBuilder& b) {b.drawPoints(SkCanvas::kPoints_PointMode,
                                               TestPointCount - 1,
                                               TestPoints);}},
      {1, 8 + TestPointCount * 8,
       [](DisplayListBuilder& b) {b.drawPoints(SkCanvas::kLines_PointMode,
                                               TestPointCount,
                                               TestPoints);}},
      {1, 8 + TestPointCount * 8,
       [](DisplayListBuilder& b) {b.drawPoints(SkCanvas::kPolygon_PointMode,
                                               TestPointCount,
                                               TestPoints);}},
    }
  },
  { "DrawVertices", {
      {1, 16, [](DisplayListBuilder& b) {b.drawVertices(TestVertices1, SkBlendMode::kSrcIn);}},
      {1, 16, [](DisplayListBuilder& b) {b.drawVertices(TestVertices1, SkBlendMode::kDstIn);}},
      {1, 16, [](DisplayListBuilder& b) {b.drawVertices(TestVertices2, SkBlendMode::kSrcIn);}},
    }
  },
  { "DrawImage", {
      {1, 40, [](DisplayListBuilder& b) {b.drawImage(TestImage1, {10, 10}, DisplayList::NearestSampling);}},
      {1, 40, [](DisplayListBuilder& b) {b.drawImage(TestImage1, {20, 10}, DisplayList::NearestSampling);}},
      {1, 40, [](DisplayListBuilder& b) {b.drawImage(TestImage1, {10, 20}, DisplayList::NearestSampling);}},
      {1, 40, [](DisplayListBuilder& b) {b.drawImage(TestImage1, {10, 10}, DisplayList::LinearSampling);}},
      {1, 40, [](DisplayListBuilder& b) {b.drawImage(TestImage2, {10, 10}, DisplayList::NearestSampling);}},
    }
  },
  { "DrawImageRect", {
      {1, 64, [](DisplayListBuilder& b) {b.drawImageRect(TestImage1, {10, 10, 20, 20}, {10, 10, 40, 40},
                                                         DisplayList::NearestSampling);}},
      {1, 64, [](DisplayListBuilder& b) {b.drawImageRect(TestImage1, {10, 10, 25, 20}, {10, 10, 40, 40},
                                                         DisplayList::NearestSampling);}},
      {1, 64, [](DisplayListBuilder& b) {b.drawImageRect(TestImage1, {10, 10, 20, 20}, {10, 10, 45, 40},
                                                         DisplayList::NearestSampling);}},
      {1, 64, [](DisplayListBuilder& b) {b.drawImageRect(TestImage1, {10, 10, 20, 20}, {10, 10, 40, 40},
                                                         DisplayList::LinearSampling);}},
      {1, 64, [](DisplayListBuilder& b) {b.drawImageRect(TestImage2, {10, 10, 20, 20}, {10, 10, 40, 40},
                                                         DisplayList::NearestSampling);}},
    }
  },
  { "DrawImageNine", {
      {1, 48, [](DisplayListBuilder& b) {b.drawImageNine(TestImage1, {10, 10, 20, 20}, {10, 10, 40, 40},
                                                         SkFilterMode::kNearest);}},
      {1, 48, [](DisplayListBuilder& b) {b.drawImageNine(TestImage1, {10, 10, 25, 20}, {10, 10, 40, 40},
                                                         SkFilterMode::kNearest);}},
      {1, 48, [](DisplayListBuilder& b) {b.drawImageNine(TestImage1, {10, 10, 20, 20}, {10, 10, 45, 40},
                                                         SkFilterMode::kNearest);}},
      {1, 48, [](DisplayListBuilder& b) {b.drawImageNine(TestImage1, {10, 10, 20, 20}, {10, 10, 40, 40},
                                                         SkFilterMode::kLinear);}},
      {1, 48, [](DisplayListBuilder& b) {b.drawImageNine(TestImage2, {10, 10, 20, 20}, {10, 10, 40, 40},
                                                         SkFilterMode::kNearest);}},
    }
  },
  // TODO(flar): Skipping DrawLattice for now
  { "DrawAtlas", {
      {1, 40 + 32 + 32, [](DisplayListBuilder& b) {
        static SkRSXform xforms[] = { {1, 0, 0, 0}, {0, 1, 0, 0} };
        static SkRect texs[] = { { 10, 10, 20, 20 }, {20, 20, 30, 30} };
        b.drawAtlas(TestImage1, xforms, texs, nullptr, 2, SkBlendMode::kSrcIn,
                    DisplayList::NearestSampling, nullptr);}},
      {1, 40 + 32 + 32, [](DisplayListBuilder& b) {
        static SkRSXform xforms[] = { {0, 1, 0, 0}, {0, 1, 0, 0} };
        static SkRect texs[] = { { 10, 10, 20, 20 }, {20, 20, 30, 30} };
        b.drawAtlas(TestImage1, xforms, texs, nullptr, 2, SkBlendMode::kSrcIn,
                    DisplayList::NearestSampling, nullptr);}},
      {1, 40 + 32 + 32, [](DisplayListBuilder& b) {
        static SkRSXform xforms[] = { {1, 0, 0, 0}, {0, 1, 0, 0} };
        static SkRect texs[] = { { 10, 10, 20, 20 }, {20, 25, 30, 30} };
        b.drawAtlas(TestImage1, xforms, texs, nullptr, 2, SkBlendMode::kSrcIn,
                    DisplayList::NearestSampling, nullptr);}},
      {1, 40 + 32 + 32, [](DisplayListBuilder& b) {
        static SkRSXform xforms[] = { {1, 0, 0, 0}, {0, 1, 0, 0} };
        static SkRect texs[] = { { 10, 10, 20, 20 }, {20, 20, 30, 30} };
        b.drawAtlas(TestImage1, xforms, texs, nullptr, 2, SkBlendMode::kSrcIn,
                    DisplayList::LinearSampling, nullptr);}},
      {1, 40 + 32 + 32, [](DisplayListBuilder& b) {
        static SkRSXform xforms[] = { {1, 0, 0, 0}, {0, 1, 0, 0} };
        static SkRect texs[] = { { 10, 10, 20, 20 }, {20, 20, 30, 30} };
        b.drawAtlas(TestImage1, xforms, texs, nullptr, 2, SkBlendMode::kDstIn,
                    DisplayList::NearestSampling, nullptr);}},
      {1, 56 + 32 + 32, [](DisplayListBuilder& b) {
        static SkRSXform xforms[] = { {1, 0, 0, 0}, {0, 1, 0, 0} };
        static SkRect texs[] = { { 10, 10, 20, 20 }, {20, 20, 30, 30} };
        static SkRect cullRect = { 0, 0, 200, 200 };
        b.drawAtlas(TestImage2, xforms, texs, nullptr, 2, SkBlendMode::kSrcIn,
                    DisplayList::NearestSampling, &cullRect);}},
      {1, 40 + 32 + 32 + 8, [](DisplayListBuilder& b) {
        static SkRSXform xforms[] = { {1, 0, 0, 0}, {0, 1, 0, 0} };
        static SkRect texs[] = { { 10, 10, 20, 20 }, {20, 20, 30, 30} };
        static SkColor colors[] = { SK_ColorBLUE, SK_ColorGREEN };
        b.drawAtlas(TestImage1, xforms, texs, colors, 2, SkBlendMode::kSrcIn,
                    DisplayList::NearestSampling, nullptr);}},
      {1, 56 + 32 + 32 + 8, [](DisplayListBuilder& b) {
        static SkRSXform xforms[] = { {1, 0, 0, 0}, {0, 1, 0, 0} };
        static SkRect texs[] = { { 10, 10, 20, 20 }, {20, 20, 30, 30} };
        static SkColor colors[] = { SK_ColorBLUE, SK_ColorGREEN };
        static SkRect cullRect = { 0, 0, 200, 200 };
        b.drawAtlas(TestImage1, xforms, texs, colors, 2, SkBlendMode::kSrcIn,
                    DisplayList::NearestSampling, &cullRect);}},
    }
  },
  { "DrawPicture", {
      {1, 16, [](DisplayListBuilder& b) {b.drawPicture(TestPicture1);}},
      {1, 16, [](DisplayListBuilder& b) {b.drawPicture(TestPicture2);}},
    }
  },
  { "DrawDisplayList", {
      {1, 16, [](DisplayListBuilder& b) {b.drawDisplayList(TestDisplayList1);}},
      {1, 16, [](DisplayListBuilder& b) {b.drawDisplayList(TestDisplayList2);}},
    }
  },
  // TODO(flar): Skipping DrawTextBlob for now
  // TODO(flar): Skipping DrawShadowRec for now
  { "DrawShadow", {
      {1, 32, [](DisplayListBuilder& b) {b.drawShadow(TestPath1, SK_ColorGREEN, 1.0, false);}},
      {1, 32, [](DisplayListBuilder& b) {b.drawShadow(TestPath2, SK_ColorGREEN, 1.0, false);}},
      {1, 32, [](DisplayListBuilder& b) {b.drawShadow(TestPath1, SK_ColorBLUE, 1.0, false);}},
      {1, 32, [](DisplayListBuilder& b) {b.drawShadow(TestPath1, SK_ColorGREEN, 2.0, false);}},
      {1, 32, [](DisplayListBuilder& b) {b.drawShadow(TestPath1, SK_ColorGREEN, 1.0, true);}},
    }
  },
};

TEST(DisplayList, SingleOpSizes) {
  for (auto& group : allGroups) {
    for (size_t i = 0; i < group.variants.size(); i++) {
      auto& invocation = group.variants[i];
      sk_sp<DisplayList> dl = invocation.build();
      auto desc = group.op_name + "(variant " + std::to_string(i) + ")";
      ASSERT_EQ(dl->opCount(), invocation.opCount) << desc;
      ASSERT_EQ(dl->bytes(), invocation.byteCount) << desc;
    }
  }
}

TEST(DisplayList, SingleOpCompares) {
  sk_sp<DisplayList> empty = DisplayListBuilder().build();
  for (auto& group : allGroups) {
    std::vector<sk_sp<DisplayList>> lists1;
    std::vector<sk_sp<DisplayList>> lists2;
    for (size_t i = 0; i < group.variants.size(); i++) {
      lists1.push_back(group.variants[i].build());
      lists2.push_back(group.variants[i].build());
      auto desc =
          group.op_name + "(variant " + std::to_string(i) + " == empty)";
      ASSERT_FALSE(lists1[i]->equals(*empty)) << desc;
      ASSERT_FALSE(lists2[i]->equals(*empty)) << desc;
      ASSERT_FALSE(empty->equals(*lists1[i])) << desc;
      ASSERT_FALSE(empty->equals(*lists2[i])) << desc;
    }
    for (size_t i = 0; i < lists1.size(); i++) {
      for (size_t j = 0; j < lists2.size(); j++) {
        auto desc = group.op_name + "(variant " + std::to_string(i) +
                    " ==? variant " + std::to_string(j) + ")";
        if (i == j) {
          ASSERT_TRUE(lists1[i]->equals(*lists2[j])) << desc;
          ASSERT_TRUE(lists2[j]->equals(*lists1[i])) << desc;
        } else {
          ASSERT_FALSE(lists1[i]->equals(*lists2[j])) << desc;
          ASSERT_FALSE(lists2[j]->equals(*lists1[i])) << desc;
        }
      }
    }
  }
}

#ifdef DL_TEST_EXPERIMENTAL
static sk_sp<DisplayList> build(size_t g_index, size_t v_index) {
  DisplayListBuilder builder;
  int opCount = 0;
  size_t byteCount = 0;
  for (size_t i = 0; i < allGroups.size(); i++) {
    int j = (i == g_index ? v_index : 0);
    if (j < 0)
      continue;
    DisplayListInvocationGroup& group = allGroups[i];
    DisplayListInvocation& invocation = group.variants[j];
    opCount += invocation.numOps;
    byteCount += invocation.numBytes;
    invocation.invoker(builder);
  }
  sk_sp<DisplayList> dl = builder.build();
  std::string name;
  if (g_index < 0) {
    name = "Default";
  } else {
    name = allGroups[g_index].op_name;
    if (v_index < 0) {
      name += " skipped";
    } else {
      name += " variant " + std::to_string(v_index);
    }
  }
  EXPECT_EQ(dl->opCount(), opCount) << name;
  EXPECT_EQ(dl->bytes(), byteCount) << name;
  return dl;
}

TEST(DisplayList, OpListEquals) {
  sk_sp<DisplayList> default_dl = build(-1, -1);
  ASSERT_EQ(default_dl.get(), default_dl.get());
  int group_count = static_cast<int>(allGroups.size());
  for (int gi = 0; gi < group_count; gi++) {
    sk_sp<DisplayList> missing_dl = build(gi, -1);
    ASSERT_EQ(missing_dl.get(), missing_dl.get());
    ASSERT_NE(default_dl.get(), missing_dl.get());
    DisplayListInvocationGroup& group = allGroups[gi];
    for (size_t vi = 0; vi < group.variants.size(); vi++) {
      sk_sp<DisplayList> variant_dl = build(gi, vi);
      ASSERT_EQ(variant_dl.get(), variant_dl.get());
      if (vi == 0) {
        ASSERT_TRUE(default_dl->equals(*variant_dl.get()));
        ASSERT_EQ(default_dl.get(), variant_dl.get());
      } else {
        ASSERT_NE(default_dl.get(), variant_dl.get());
      }
      ASSERT_NE(missing_dl.get(), variant_dl.get());
    }
  }
}
#endif  // DL_TEST_EXPERIMENTAL

}  // namespace testing
}  // namespace flutter
