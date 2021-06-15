// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/flow/display_list_canvas.h"

#include "third_party/skia/include/core/SkColor.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkVertices.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/effects/SkImageFilters.h"

#include <cmath>

#include "gtest/gtest.h"

namespace flutter {
namespace testing {

constexpr int TestWidth = 200;
constexpr int TestHeight = 200;
constexpr int RenderWidth = 100;
constexpr int RenderHeight = 100;
constexpr int RenderLeft = (TestWidth - RenderWidth) / 2;
constexpr int RenderTop = (TestHeight - RenderHeight) / 2;
constexpr int RenderRight = RenderLeft + RenderWidth;
constexpr int RenderBottom = RenderTop + RenderHeight;
constexpr int RenderCenterX = (RenderLeft + RenderRight) / 2;
constexpr int RenderCenterY = (RenderTop + RenderBottom) / 2;
constexpr SkScalar RenderRadius = std::min(RenderWidth, RenderHeight) / 2.0;
constexpr SkScalar RenderCornerRadius = RenderRadius / 5.0;

constexpr SkPoint TestCenter = SkPoint::Make(TestWidth / 2, TestHeight / 2);
constexpr SkRect TestBounds = SkRect::MakeWH(TestWidth, TestHeight);
constexpr SkRect RenderBounds =
    SkRect::MakeLTRB(RenderLeft, RenderTop, RenderRight, RenderBottom);

static bool skipTheColorFilter = false;

class CanvasCompareTester {
 public:
  typedef const std::function<void(SkCanvas*, SkPaint&)> CvRenderer;
  typedef const std::function<void(DisplayListBuilder&)> DlRenderer;

  static void renderAll(CvRenderer& cv_renderer, DlRenderer& dl_renderer) {
    renderWithAttributes(cv_renderer, dl_renderer);
    renderWithTransforms(cv_renderer, dl_renderer);
    renderWithClips(cv_renderer, dl_renderer);
  }

  static void renderWithAttributes(CvRenderer& cv_renderer,
                                   DlRenderer& dl_renderer) {
    renderWith([=](SkCanvas*, SkPaint& p) {},  //
               [=](DisplayListBuilder& d) {},  //
               cv_renderer, dl_renderer, "Base Test");

    renderWith([=](SkCanvas*, SkPaint& p) { p.setAntiAlias(true); },  //
               [=](DisplayListBuilder& b) { b.setAA(true); },         //
               cv_renderer, dl_renderer, "AA == True");
    renderWith([=](SkCanvas*, SkPaint& p) { p.setAntiAlias(false); },  //
               [=](DisplayListBuilder& b) { b.setAA(false); },         //
               cv_renderer, dl_renderer, "AA == False");

    // Not testing setInvertColors here because there is no SkPaint version

    renderWith([=](SkCanvas*, SkPaint& p) { p.setDither(true); },  //
               [=](DisplayListBuilder& b) { b.setDither(true); },  //
               cv_renderer, dl_renderer, "Dither == True");
    renderWith([=](SkCanvas*, SkPaint& p) { p.setDither(false); },  //
               [=](DisplayListBuilder& b) { b.setDither(false); },  //
               cv_renderer, dl_renderer, "Dither = False");

    renderWith([=](SkCanvas*, SkPaint& p) { p.setColor(SK_ColorBLUE); },  //
               [=](DisplayListBuilder& b) { b.setColor(SK_ColorBLUE); },  //
               cv_renderer, dl_renderer, "Color == Blue");
    renderWith([=](SkCanvas*, SkPaint& p) { p.setColor(SK_ColorGREEN); },  //
               [=](DisplayListBuilder& b) { b.setColor(SK_ColorGREEN); },  //
               cv_renderer, dl_renderer, "Color == Green");

    renderWithStrokes(cv_renderer, dl_renderer);

    // Not testing FilterQuality here because there is no SkPaint version

    {
      // half opaque cyan
      SkColor blendableColor = SkColorSetARGB(0x7f, 0x00, 0xff, 0xff);
      SkColor bg = SK_ColorWHITE;

      renderWith(
          [=](SkCanvas*, SkPaint& p) {
            p.setBlendMode(SkBlendMode::kSrcIn);
            p.setColor(blendableColor);
          },
          [=](DisplayListBuilder& b) {
            b.setBlendMode(SkBlendMode::kSrcIn);
            b.setColor(blendableColor);
          },
          cv_renderer, dl_renderer, "Blend == SrcIn", &bg);
      renderWith(
          [=](SkCanvas*, SkPaint& p) {
            p.setBlendMode(SkBlendMode::kDstIn);
            p.setColor(blendableColor);
          },
          [=](DisplayListBuilder& b) {
            b.setBlendMode(SkBlendMode::kDstIn);
            b.setColor(blendableColor);
          },
          cv_renderer, dl_renderer, "Blend == DstIn", &bg);
    }

    {
      sk_sp<SkImageFilter> filter =
          SkImageFilters::Blur(5.0, 5.0, SkTileMode::kDecal, nullptr, nullptr);
      {
        renderWith([=](SkCanvas*, SkPaint& p) { p.setImageFilter(filter); },
                   [=](DisplayListBuilder& b) { b.setImageFilter(filter); },
                   cv_renderer, dl_renderer, "ImageFilter == Decal Blur 5");
      }
      ASSERT_TRUE(filter->unique()) << "ImageFilter Cleanup";
    }

    if (skipTheColorFilter) {
      // drawVertices + ColorFilter outputs nothing on the CPU renderer
      FML_LOG(ERROR) << "Skipping the ColorFilter test";
    } else {
      constexpr float rotate_color_matrix[20] = {
          0, 1, 0, 0, 0,  //
          0, 0, 1, 0, 0,  //
          1, 0, 0, 0, 0,  //
          0, 0, 0, 1, 0,  //
      };
      sk_sp<SkColorFilter> filter = SkColorFilters::Matrix(rotate_color_matrix);
      {
        SkColor bg = SK_ColorWHITE;
        renderWith(
            [=](SkCanvas*, SkPaint& p) {
              p.setColor(SK_ColorYELLOW);
              p.setColorFilter(filter);
            },
            [=](DisplayListBuilder& b) {
              b.setColor(SK_ColorYELLOW);
              b.setColorFilter(filter);
            },
            cv_renderer, dl_renderer, "ColorFilter == RotateRGB", &bg);
      }
      ASSERT_TRUE(filter->unique()) << "ColorFilter Cleanup";
    }

    {
      sk_sp<SkMaskFilter> filter =
          SkMaskFilter::MakeBlur(kNormal_SkBlurStyle, 5.0);
      {
        renderWith([=](SkCanvas*, SkPaint& p) { p.setMaskFilter(filter); },
                   [=](DisplayListBuilder& b) { b.setMaskFilter(filter); },
                   cv_renderer, dl_renderer, "MaskFilter == Blur 5");
      }
      ASSERT_TRUE(filter->unique()) << "MaskFilter Cleanup";
      {
        renderWith([=](SkCanvas*, SkPaint& p) { p.setMaskFilter(filter); },
                   [=](DisplayListBuilder& b) {
                     b.setMaskBlurFilter(kNormal_SkBlurStyle, 5.0);
                   },
                   cv_renderer, dl_renderer, "MaskFilter == Blur(Normal, 5.0)");
      }
      ASSERT_TRUE(filter->unique()) << "MaskFilter Cleanup";
    }

    {
      SkPoint end_points[] = {
          SkPoint::Make(RenderBounds.fLeft, RenderBounds.fTop),
          SkPoint::Make(RenderBounds.fRight, RenderBounds.fBottom),
      };
      SkColor colors[] = {
          SK_ColorGREEN,
          SK_ColorYELLOW,
          SK_ColorBLUE,
      };
      float stops[] = {
          0.0,
          0.5,
          1.0,
      };
      sk_sp<SkShader> shader = SkGradientShader::MakeLinear(
          end_points, colors, stops, 3, SkTileMode::kMirror, 0, nullptr);
      {
        renderWith([=](SkCanvas*, SkPaint& p) { p.setShader(shader); },
                   [=](DisplayListBuilder& b) { b.setShader(shader); },
                   cv_renderer, dl_renderer, "LinearGradient GYB");
      }
      ASSERT_TRUE(shader->unique()) << "Shader Cleanup";
    }
  }

  static void renderWithStrokes(CvRenderer& cv_renderer,
                                DlRenderer& dl_renderer) {
    renderWith(
        [=](SkCanvas*, SkPaint& p) { p.setStyle(SkPaint::kFill_Style); },
        [=](DisplayListBuilder& b) { b.setDrawStyle(SkPaint::kFill_Style); },
        cv_renderer, dl_renderer, "Fill");
    renderWith(
        [=](SkCanvas*, SkPaint& p) { p.setStyle(SkPaint::kStroke_Style); },
        [=](DisplayListBuilder& b) { b.setDrawStyle(SkPaint::kStroke_Style); },
        cv_renderer, dl_renderer, "Stroke + defaults");

    renderWith(
        [=](SkCanvas*, SkPaint& p) {
          p.setStyle(SkPaint::kFill_Style);
          p.setStrokeWidth(10.0);
        },
        [=](DisplayListBuilder& b) {
          b.setDrawStyle(SkPaint::kFill_Style);
          b.setStrokeWidth(10.0);
        },
        cv_renderer, dl_renderer, "Fill + unnecessary StrokeWidth 10");

    renderWith(
        [=](SkCanvas*, SkPaint& p) {
          p.setStyle(SkPaint::kStroke_Style);
          p.setStrokeWidth(10.0);
        },
        [=](DisplayListBuilder& b) {
          b.setDrawStyle(SkPaint::kStroke_Style);
          b.setStrokeWidth(10.0);
        },
        cv_renderer, dl_renderer, "Stroke Width 10");
    renderWith(
        [=](SkCanvas*, SkPaint& p) {
          p.setStyle(SkPaint::kStroke_Style);
          p.setStrokeWidth(5.0);
        },
        [=](DisplayListBuilder& b) {
          b.setDrawStyle(SkPaint::kStroke_Style);
          b.setStrokeWidth(5.0);
        },
        cv_renderer, dl_renderer, "Stroke Width 5");

    renderWith(
        [=](SkCanvas*, SkPaint& p) {
          p.setStyle(SkPaint::kStroke_Style);
          p.setStrokeWidth(5.0);
          p.setStrokeCap(SkPaint::kButt_Cap);
        },
        [=](DisplayListBuilder& b) {
          b.setDrawStyle(SkPaint::kStroke_Style);
          b.setStrokeWidth(5.0);
          b.setCaps(SkPaint::kButt_Cap);
        },
        cv_renderer, dl_renderer, "Stroke Width 5, Butt Cap");
    renderWith(
        [=](SkCanvas*, SkPaint& p) {
          p.setStyle(SkPaint::kStroke_Style);
          p.setStrokeWidth(5.0);
          p.setStrokeCap(SkPaint::kRound_Cap);
        },
        [=](DisplayListBuilder& b) {
          b.setDrawStyle(SkPaint::kStroke_Style);
          b.setStrokeWidth(5.0);
          b.setCaps(SkPaint::kRound_Cap);
        },
        cv_renderer, dl_renderer, "Stroke Width 5, Round Cap");

    renderWith(
        [=](SkCanvas*, SkPaint& p) {
          p.setStyle(SkPaint::kStroke_Style);
          p.setStrokeWidth(5.0);
          p.setStrokeJoin(SkPaint::kBevel_Join);
        },
        [=](DisplayListBuilder& b) {
          b.setDrawStyle(SkPaint::kStroke_Style);
          b.setStrokeWidth(5.0);
          b.setJoins(SkPaint::kBevel_Join);
        },
        cv_renderer, dl_renderer, "Stroke Width 5, Bevel Join");
    renderWith(
        [=](SkCanvas*, SkPaint& p) {
          p.setStyle(SkPaint::kStroke_Style);
          p.setStrokeWidth(5.0);
          p.setStrokeJoin(SkPaint::kRound_Join);
        },
        [=](DisplayListBuilder& b) {
          b.setDrawStyle(SkPaint::kStroke_Style);
          b.setStrokeWidth(5.0);
          b.setJoins(SkPaint::kRound_Join);
        },
        cv_renderer, dl_renderer, "Stroke Width 5, Round Join");

    renderWith(
        [=](SkCanvas*, SkPaint& p) {
          p.setStyle(SkPaint::kStroke_Style);
          p.setStrokeWidth(5.0);
          p.setStrokeMiter(100.0);
          p.setStrokeJoin(SkPaint::kMiter_Join);
        },
        [=](DisplayListBuilder& b) {
          b.setDrawStyle(SkPaint::kStroke_Style);
          b.setStrokeWidth(5.0);
          b.setMiterLimit(100.0);
          b.setJoins(SkPaint::kMiter_Join);
        },
        cv_renderer, dl_renderer, "Stroke Width 5, Miter 100");

    renderWith(
        [=](SkCanvas*, SkPaint& p) {
          p.setStyle(SkPaint::kStroke_Style);
          p.setStrokeWidth(5.0);
          p.setStrokeMiter(0.0);
          p.setStrokeJoin(SkPaint::kMiter_Join);
        },
        [=](DisplayListBuilder& b) {
          b.setDrawStyle(SkPaint::kStroke_Style);
          b.setStrokeWidth(5.0);
          b.setMiterLimit(0.0);
          b.setJoins(SkPaint::kMiter_Join);
        },
        cv_renderer, dl_renderer, "Stroke Width 5, Miter 0");
  }

  static void renderWithTransforms(CvRenderer& cv_renderer,
                                   DlRenderer& dl_renderer) {
    renderWith([=](SkCanvas* c, SkPaint&) { c->translate(5, 10); },  //
               [=](DisplayListBuilder& b) { b.translate(5, 10); },   //
               cv_renderer, dl_renderer, "Translate 5, 10");
    renderWith([=](SkCanvas* c, SkPaint&) { c->scale(0.95, 0.95); },  //
               [=](DisplayListBuilder& b) { b.scale(0.95, 0.95); },   //
               cv_renderer, dl_renderer, "Scale 95%");
    renderWith([=](SkCanvas* c, SkPaint&) { c->rotate(5); },  //
               [=](DisplayListBuilder& b) { b.rotate(5); },   //
               cv_renderer, dl_renderer, "Rotate 5 degrees");
    renderWith([=](SkCanvas* c, SkPaint&) { c->skew(0.05, 0.05); },  //
               [=](DisplayListBuilder& b) { b.skew(0.05, 0.05); },   //
               cv_renderer, dl_renderer, "Skew 5%");
  }

  static void renderWithClips(CvRenderer& cv_renderer,
                              DlRenderer& dl_renderer) {
    renderWith(
        [=](SkCanvas* c, SkPaint&) {
          c->clipRect(RenderBounds.makeInset(25.5, 25.5),  //
                      SkClipOp::kIntersect, false);
        },
        [=](DisplayListBuilder& b) {
          b.clipRect(RenderBounds.makeInset(25.5, 25.5),  //
                     false, SkClipOp::kIntersect);
        },
        cv_renderer, dl_renderer, "Hard ClipRect inset by 25.5");
    renderWith(
        [=](SkCanvas* c, SkPaint&) {
          c->clipRect(RenderBounds.makeInset(25.5, 25.5),  //
                      SkClipOp::kIntersect, true);
        },
        [=](DisplayListBuilder& b) {
          b.clipRect(RenderBounds.makeInset(25.5, 25.5),  //
                     true, SkClipOp::kIntersect);
        },
        cv_renderer, dl_renderer, "AA ClipRect inset by 25.5");
    renderWith(
        [=](SkCanvas* c, SkPaint&) {
          c->clipRect(RenderBounds.makeInset(25.5, 25.5),  //
                      SkClipOp::kDifference, false);
        },
        [=](DisplayListBuilder& b) {
          b.clipRect(RenderBounds.makeInset(25.5, 25.5),  //
                     false, SkClipOp::kDifference);
        },
        cv_renderer, dl_renderer, "Hard ClipRect Diff, inset by 25.5");
  }

  static SkRect getSkBounds(CvRenderer& cv_setup, CvRenderer& cv_render) {
    SkPictureRecorder recorder;
    SkRTreeFactory rtree_factory;
    SkCanvas* cv = recorder.beginRecording(TestBounds, &rtree_factory);
    SkPaint p;
    cv_setup(cv, p);
    cv_render(cv, p);
    return recorder.finishRecordingAsPicture()->cullRect();
  }

  static void renderWith(CvRenderer& cv_setup,
                         DlRenderer& dl_setup,
                         CvRenderer& cv_render,
                         DlRenderer& dl_render,
                         const std::string info,
                         const SkColor* bg = nullptr) {
    // surface1 is direct rendering via SkCanvas to SkSurface
    // DisplayList mechanisms are not involved in this operation
    sk_sp<SkSurface> ref_surface = makeSurface(bg);
    SkPaint paint1;
    cv_setup(ref_surface->getCanvas(), paint1);
    cv_render(ref_surface->getCanvas(), paint1);
    SkRect ref_bounds = getSkBounds(cv_setup, cv_render);
    SkPixmap ref_pixels;
    ASSERT_TRUE(ref_surface->peekPixels(&ref_pixels)) << info;
    ASSERT_EQ(ref_pixels.width(), TestWidth) << info;
    ASSERT_EQ(ref_pixels.height(), TestHeight) << info;
    ASSERT_EQ(ref_pixels.info().bytesPerPixel(), 4) << info;
    checkPixels(&ref_pixels, ref_bounds, info, bg);

    {
      // This sequence plays the provided equivalently constructed
      // DisplayList onto the SkCanvas of the surface
      // DisplayList => direct rendering
      sk_sp<SkSurface> test_surface = makeSurface(bg);
      DisplayListBuilder builder(TestBounds);
      dl_setup(builder);
      dl_render(builder);
      sk_sp<DisplayList> display_list = builder.build();
      SkRect dl_bounds = display_list->bounds();
#ifdef DISPLAY_LIST_BOUNDS_ACCURACY_CHECKING
      if (dl_bounds != ref_bounds) {
        FML_LOG(ERROR) << "For " << info;
        FML_LOG(ERROR) << "ref: " << ref_bounds.fLeft << ", " << ref_bounds.fTop
                       << " => " << ref_bounds.fRight << ", "
                       << ref_bounds.fBottom;
        FML_LOG(ERROR) << "dl: " << dl_bounds.fLeft << ", " << dl_bounds.fTop
                       << " => " << dl_bounds.fRight << ", "
                       << dl_bounds.fBottom;
        if (!dl_bounds.contains(ref_bounds)) {
          FML_LOG(ERROR) << "DisplayList bounds are too small!";
        }
      }
#endif  // DISPLAY_LIST_BOUNDS_ACCURACY_CHECKING
      // This sometimes triggers, but when it triggers and I examine
      // the ref_bounds, they are always unnecessarily large and
      // since the pixel OOB tests in the compare method do not
      // trigger, we will trust the DL bounds.
      // EXPECT_TRUE(dl_bounds.contains(ref_bounds)) << info;
      display_list->renderTo(test_surface->getCanvas());
      compareToReference(test_surface.get(), &ref_pixels, info + " (DL render)",
                         &dl_bounds, bg);
    }

    {
      // This sequence renders SkCanvas calls to a DisplayList and then
      // plays them back on SkCanvas to SkSurface
      // SkCanvas calls => DisplayList => rendering
      sk_sp<SkSurface> test_surface = makeSurface(bg);
      DisplayListCanvasRecorder dl_recorder(TestBounds);
      SkPaint test_paint;
      cv_setup(&dl_recorder, test_paint);
      cv_render(&dl_recorder, test_paint);
      dl_recorder.builder()->build()->renderTo(test_surface->getCanvas());
      compareToReference(test_surface.get(), &ref_pixels,
                         info + " (Sk->DL render)", nullptr, nullptr);
    }
  }

  static void checkPixels(SkPixmap* ref_pixels,
                          SkRect ref_bounds,
                          const std::string info,
                          const SkColor* bg) {
    SkPMColor untouched = (bg) ? SkPreMultiplyColor(*bg) : 0;
    int pixels_touched = 0;
    int pixels_oob = 0;
    for (int y = 0; y < TestHeight; y++) {
      const uint32_t* ref_row = ref_pixels->addr32(0, y);
      for (int x = 0; x < TestWidth; x++) {
        if (ref_row[x] != untouched) {
          pixels_touched++;
          if (!ref_bounds.intersects(SkRect::MakeXYWH(x, y, 1, 1))) {
            pixels_oob++;
          }
        }
      }
    }
    ASSERT_EQ(pixels_oob, 0) << info;
    ASSERT_GT(pixels_touched, 0) << info;
  }

  static void compareToReference(SkSurface* test_surface,
                                 SkPixmap* reference,
                                 const std::string info,
                                 SkRect* bounds,
                                 const SkColor* bg) {
    SkPMColor untouched = (bg) ? SkPreMultiplyColor(*bg) : 0;
    SkPixmap test_pixels;
    ASSERT_TRUE(test_surface->peekPixels(&test_pixels)) << info;
    ASSERT_EQ(test_pixels.width(), TestWidth) << info;
    ASSERT_EQ(test_pixels.height(), TestHeight) << info;
    ASSERT_EQ(test_pixels.info().bytesPerPixel(), 4) << info;

    int pixels_different = 0;
    int pixels_oob = 0;
    int minX = TestWidth;
    int minY = TestWidth;
    int maxX = 0;
    int maxY = 0;
    for (int y = 0; y < TestHeight; y++) {
      const uint32_t* ref_row = reference->addr32(0, y);
      const uint32_t* test_row = test_pixels.addr32(0, y);
      for (int x = 0; x < TestWidth; x++) {
        if (bounds && test_row[x] != untouched) {
          if (minX > x)
            minX = x;
          if (minY > y)
            minY = y;
          if (maxX < x)
            maxX = x;
          if (maxY < y)
            maxY = y;
          if (!bounds->intersects(SkRect::MakeXYWH(x, y, 1, 1))) {
            pixels_oob++;
          }
        }
        if (test_row[x] != ref_row[x]) {
          pixels_different++;
        }
      }
    }
#ifdef DISPLAY_LIST_BOUNDS_ACCURACY_CHECKING
    if (bounds && *bounds != SkRect::MakeLTRB(minX, minY, maxX + 1, maxY + 1)) {
      FML_LOG(ERROR) << "inaccurate bounds for " << info;
      FML_LOG(ERROR) << "dl: " << bounds->fLeft << ", " << bounds->fTop
                     << " => " << bounds->fRight << ", " << bounds->fBottom;
      FML_LOG(ERROR) << "pixels: " << minX << ", " << minY << " => "
                     << (maxX + 1) << ", " << (maxY + 1);
    }
#endif  // DISPLAY_LIST_BOUNDS_ACCURACY_CHECKING
    ASSERT_EQ(pixels_oob, 0) << info;
    ASSERT_EQ(pixels_different, 0) << info;
  }

  static sk_sp<SkSurface> makeSurface(const SkColor* bg) {
    sk_sp<SkSurface> surface =
        SkSurface::MakeRasterN32Premul(TestWidth, TestHeight);
    if (bg) {
      surface->getCanvas()->drawColor(*bg);
    }
    return surface;
  }

  static const sk_sp<SkImage> testImage;
  static const sk_sp<SkImage> makeTestImage() {
    sk_sp<SkSurface> surface =
        SkSurface::MakeRasterN32Premul(RenderWidth, RenderHeight);
    SkCanvas* canvas = surface->getCanvas();
    SkPaint p0, p1;
    p0.setStyle(SkPaint::kFill_Style);
    p0.setColor(SK_ColorGREEN);
    p1.setStyle(SkPaint::kFill_Style);
    p1.setColor(SK_ColorBLUE);
    // Some pixels need some transparency for DstIn testing
    p1.setAlpha(128);
    int cbdim = 5;
    for (int y = 0; y < RenderHeight; y += cbdim) {
      for (int x = 0; x < RenderWidth; x += cbdim) {
        SkPaint& cellp = ((x + y) & 1) == 0 ? p0 : p1;
        canvas->drawRect(SkRect::MakeXYWH(x, y, cbdim, cbdim), cellp);
      }
    }
    return surface->makeImageSnapshot();
  }
};

const sk_sp<SkImage> CanvasCompareTester::testImage =
    CanvasCompareTester::makeTestImage();

TEST(DisplayListCanvas, DrawPaint) {
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawPaint(paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawPaint();
      });
}

TEST(DisplayListCanvas, DrawColor) {
  CanvasCompareTester::renderWith(             //
      [=](SkCanvas*, SkPaint& p) {},           //
      [=](DisplayListBuilder& b) {},           //
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawColor(SK_ColorMAGENTA);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawColor(SK_ColorMAGENTA, SkBlendMode::kSrcOver);
      },
      "No SkPaint");
  CanvasCompareTester::renderWithTransforms(   //
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawColor(SK_ColorMAGENTA);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawColor(SK_ColorMAGENTA, SkBlendMode::kSrcOver);
      });
  CanvasCompareTester::renderWithClips(        //
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawColor(SK_ColorMAGENTA);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawColor(SK_ColorMAGENTA, SkBlendMode::kSrcOver);
      });
}

TEST(DisplayListCanvas, DrawLine) {
  SkRect rect = RenderBounds.makeInset(20, 20);
  SkPoint p1 = SkPoint::Make(rect.fLeft, rect.fTop);
  SkPoint p2 = SkPoint::Make(rect.fRight, rect.fBottom);

  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawLine(p1, p2, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawLine(p1, p2);
      });
}

TEST(DisplayListCanvas, DrawRect) {
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawRect(RenderBounds, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawRect(RenderBounds);
      });
}

TEST(DisplayListCanvas, DrawOval) {
  SkRect rect = RenderBounds.makeInset(0, 10);

  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawOval(rect, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawOval(rect);
      });
}

TEST(DisplayListCanvas, DrawCircle) {
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawCircle(TestCenter, RenderRadius, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawCircle(TestCenter, RenderRadius);
      });
}

TEST(DisplayListCanvas, DrawRRect) {
  SkRRect rrect =
      SkRRect::MakeRectXY(RenderBounds, RenderCornerRadius, RenderCornerRadius);
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawRRect(rrect, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawRRect(rrect);
      });
}

TEST(DisplayListCanvas, DrawDRRect) {
  SkRRect outer =
      SkRRect::MakeRectXY(RenderBounds, RenderCornerRadius, RenderCornerRadius);
  SkRect innerBounds = RenderBounds.makeInset(30.0, 30.0);
  SkRRect inner =
      SkRRect::MakeRectXY(innerBounds, RenderCornerRadius, RenderCornerRadius);
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawDRRect(outer, inner, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawDRRect(outer, inner);
      });
}

TEST(DisplayListCanvas, DrawPath) {
  SkPath path;
  path.moveTo(RenderCenterX, RenderTop);
  path.lineTo(RenderRight, RenderBottom);
  path.lineTo(RenderLeft, RenderCenterY);
  path.lineTo(RenderRight, RenderCenterY);
  path.lineTo(RenderLeft, RenderBottom);
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawPath(path, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawPath(path);
      });
}

TEST(DisplayListCanvas, DrawArc) {
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawArc(RenderBounds, 30, 270, false, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawArc(RenderBounds, 30, 270, false);
      });
}

TEST(DisplayListCanvas, DrawArcCenter) {
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawArc(RenderBounds, 30, 270, true, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawArc(RenderBounds, 30, 270, true);
      });
}

TEST(DisplayListCanvas, DrawPointsAsPoints) {
  const SkPoint points[] = {
      SkPoint::Make(RenderLeft, RenderTop),
      SkPoint::Make(RenderRight, RenderBottom),
      SkPoint::Make(RenderRight, RenderTop),
      SkPoint::Make(RenderLeft, RenderBottom),
      SkPoint::Make(RenderCenterX, RenderCenterY),
  };
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawPoints(SkCanvas::kPoints_PointMode, 5, points, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawPoints(SkCanvas::kPoints_PointMode, 5, points);
      });
}

TEST(DisplayListCanvas, DrawPointsAsLines) {
  const SkPoint points[] = {
      SkPoint::Make(RenderLeft, RenderTop),
      SkPoint::Make(RenderRight, RenderBottom),
      SkPoint::Make(RenderRight, RenderTop),
      SkPoint::Make(RenderLeft, RenderBottom),
  };
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawPoints(SkCanvas::kLines_PointMode, 4, points, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawPoints(SkCanvas::kLines_PointMode, 4, points);
      });
}

TEST(DisplayListCanvas, DrawPointsAsPolygon) {
  const SkPoint points[] = {
      SkPoint::Make(RenderLeft, RenderTop),
      SkPoint::Make(RenderRight, RenderBottom),
      SkPoint::Make(RenderRight, RenderTop),
      SkPoint::Make(RenderLeft, RenderBottom),
  };
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawPoints(SkCanvas::kPolygon_PointMode, 4, points, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawPoints(SkCanvas::kPolygon_PointMode, 4, points);
      });
}

TEST(DisplayListCanvas, DrawVerticesWithColors) {
  skipTheColorFilter = true;
  const SkPoint pts[3] = {
      SkPoint::Make(RenderCenterX, RenderTop),
      SkPoint::Make(RenderLeft, RenderBottom),
      SkPoint::Make(RenderRight, RenderBottom),
  };
  const SkColor colors[3] = {SK_ColorRED, SK_ColorBLUE, SK_ColorGREEN};
  const sk_sp<SkVertices> vertices = SkVertices::MakeCopy(
      SkVertices::kTriangles_VertexMode, 3, pts, nullptr, colors);
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawVertices(vertices.get(), SkBlendMode::kSrcOver, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawVertices(vertices, SkBlendMode::kSrcOver);
      });
  // Since there are no ASSERT macros above here, this line will execute
  skipTheColorFilter = false;
  ASSERT_TRUE(vertices->unique());
}

TEST(DisplayListCanvas, DrawVerticesWithImage) {
  skipTheColorFilter = true;
  const SkPoint pts[3] = {
      SkPoint::Make(RenderCenterX, RenderTop),
      SkPoint::Make(RenderLeft, RenderBottom),
      SkPoint::Make(RenderRight, RenderBottom),
  };
  const SkPoint tex[3] = {
      SkPoint::Make(RenderWidth / 2.0, 0),
      SkPoint::Make(0, RenderHeight),
      SkPoint::Make(RenderWidth, RenderHeight),
  };
  const sk_sp<SkVertices> vertices = SkVertices::MakeCopy(
      SkVertices::kTriangles_VertexMode, 3, pts, tex, nullptr);
  const sk_sp<SkShader> shader = CanvasCompareTester::testImage->makeShader(
      SkTileMode::kRepeat, SkTileMode::kRepeat, SkSamplingOptions());
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        paint.setShader(shader);
        canvas->drawVertices(vertices.get(), SkBlendMode::kSrcOver, paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.setShader(shader);
        builder.drawVertices(vertices, SkBlendMode::kSrcOver);
      });
  // Since there are no ASSERT macros above here, this line will execute
  skipTheColorFilter = false;
  ASSERT_TRUE(vertices->unique());
  ASSERT_TRUE(shader->unique());
}

TEST(DisplayListCanvas, DrawImageNearest) {
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawImage(CanvasCompareTester::testImage, RenderLeft, RenderTop,
                          DisplayList::NearestSampling, &paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawImage(CanvasCompareTester::testImage,
                          SkPoint::Make(RenderLeft, RenderTop),
                          DisplayList::NearestSampling);
      });
}

TEST(DisplayListCanvas, DrawImageLinear) {
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawImage(CanvasCompareTester::testImage, RenderLeft, RenderTop,
                          DisplayList::LinearSampling, &paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawImage(CanvasCompareTester::testImage,
                          SkPoint::Make(RenderLeft, RenderTop),
                          DisplayList::LinearSampling);
      });
}

TEST(DisplayListCanvas, DrawImageRectNearest) {
  SkRect src = SkRect::MakeIWH(RenderWidth, RenderHeight).makeInset(5, 5);
  SkRect dst = RenderBounds.makeInset(15.5, 10.5);
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawImageRect(CanvasCompareTester::testImage, src, dst,
                              DisplayList::NearestSampling, &paint,
                              SkCanvas::kFast_SrcRectConstraint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawImageRect(CanvasCompareTester::testImage, src, dst,
                              DisplayList::NearestSampling);
      });
}

TEST(DisplayListCanvas, DrawImageRectLinear) {
  SkRect src = SkRect::MakeIWH(RenderWidth, RenderHeight).makeInset(5, 5);
  SkRect dst = RenderBounds.makeInset(15.5, 10.5);
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawImageRect(CanvasCompareTester::testImage, src, dst,
                              DisplayList::LinearSampling, &paint,
                              SkCanvas::kFast_SrcRectConstraint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawImageRect(CanvasCompareTester::testImage, src, dst,
                              DisplayList::LinearSampling);
      });
}

TEST(DisplayListCanvas, DrawImageNineNearest) {
  SkIRect src = SkIRect::MakeWH(RenderWidth, RenderHeight).makeInset(5, 5);
  SkRect dst = RenderBounds.makeInset(15.5, 10.5);
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawImageNine(CanvasCompareTester::testImage.get(), src, dst,
                              SkFilterMode::kNearest, &paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawImageNine(CanvasCompareTester::testImage, src, dst,
                              SkFilterMode::kNearest);
      });
}

TEST(DisplayListCanvas, DrawImageNineLinear) {
  SkIRect src = SkIRect::MakeWH(RenderWidth, RenderHeight).makeInset(5, 5);
  SkRect dst = RenderBounds.makeInset(15.5, 10.5);
  CanvasCompareTester::renderAll(
      [=](SkCanvas* canvas, SkPaint& paint) {  //
        canvas->drawImageNine(CanvasCompareTester::testImage.get(), src, dst,
                              SkFilterMode::kLinear, &paint);
      },
      [=](DisplayListBuilder& builder) {  //
        builder.drawImageNine(CanvasCompareTester::testImage, src, dst,
                              SkFilterMode::kLinear);
      });
}

}  // namespace testing
}  // namespace flutter
