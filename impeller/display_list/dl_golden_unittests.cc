// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "impeller/display_list/dl_golden_unittests.h"

#include "flutter/display_list/dl_builder.h"
#include "flutter/testing/testing.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

using impeller::PlaygroundBackend;
using impeller::PlaygroundTest;

INSTANTIATE_PLAYGROUND_SUITE(DlGoldenTest);

TEST_P(DlGoldenTest, CanDrawPaint) {
  auto draw = [](DlCanvas* canvas,
                 const std::vector<std::unique_ptr<DlImage>>& images) {
    canvas->Scale(0.2, 0.2);
    DlPaint paint;
    paint.setColor(DlColor::kCyan());
    canvas->DrawPaint(paint);
  };

  DisplayListBuilder builder;
  draw(&builder, /*images=*/{});

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(DlGoldenTest, CanRenderImage) {
  auto draw = [](DlCanvas* canvas, const std::vector<sk_sp<DlImage>>& images) {
    FML_CHECK(images.size() >= 1);
    DlPaint paint;
    paint.setColor(DlColor::kRed());
    canvas->DrawImage(images[0], SkPoint::Make(100.0, 100.0),
                      DlImageSampling::kLinear, &paint);
  };

  DisplayListBuilder builder;
  std::vector<sk_sp<DlImage>> images;
  images.emplace_back(CreateDlImageForFixture("kalimba.jpg"));
  draw(&builder, images);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

TEST_P(DlGoldenTest, Bug147807) {
  auto draw = [](DlCanvas* canvas, const std::vector<sk_sp<DlImage>>& images) {
    canvas->Transform2DAffine(2, 0, 0, 0, 2, 0);
    DlPaint paint;
    paint.setColor(DlColor(0xfffef7ff));
    canvas->DrawRect(SkRect::MakeLTRB(0, 0, 375, 667), paint);
    paint.setColor(DlColor(0xffff9800));
    canvas->DrawRect(SkRect::MakeLTRB(0, 0, 187.5, 333.5), paint);
    paint.setColor(DlColor(0xff9c27b0));
    canvas->DrawRect(SkRect::MakeLTRB(187.5, 0, 375, 333.5), paint);
    paint.setColor(DlColor(0xff4caf50));
    canvas->DrawRect(SkRect::MakeLTRB(0, 333.5, 187.5, 667), paint);
    paint.setColor(DlColor(0xfff44336));
    canvas->DrawRect(SkRect::MakeLTRB(187.5, 333.5, 375, 667), paint);
    canvas->Save();
    {
      canvas->Transform2DAffine(1, 2.449293705170336e-16, -1.70530256582424e-13,
                                -2.449293705170336e-16, 1,
                                1.13686837721616e-13);
      canvas->DrawShadow(SkPath().addRect(SkRect::MakeLTRB(303, 595, 359, 651)),
                         DlColor(0xffebddff), 6, false, 2);
      canvas->DrawPath(SkPath().addRect(SkRect::MakeLTRB(303, 595, 359, 651)),
                       DlPaint().setColor(DlColor(0xffebddff)));
      canvas->Save();
      {
        canvas->Translate(303, 595);
        canvas->ClipRRect(SkRRect::MakeRect(SkRect::MakeLTRB(0, 0, 56, 56)),
                          DlCanvas::ClipOp::kIntersect, true);
        canvas->Save();
        {
          canvas->ClipRRect(SkRRect::MakeOval(SkRect::MakeLTRB(0, 0, 56, 56)),
                            DlCanvas::ClipOp::kIntersect, true);
          canvas->DrawCircle(
              SkPoint::Make(21.55413055419922, 25.02498245239258),
              12.15186500549316, DlPaint().setColor(DlColor(0x6230f46)));
        }
        canvas->Restore();
        canvas->Save();
        {
          canvas->ClipRRect(SkRRect::MakeOval(SkRect::MakeLTRB(0, 0, 56, 56)),
                            DlCanvas::ClipOp::kIntersect, true);
          canvas->DrawRect(SkRect::MakeLTRB(0, 0, 56, 56),
                           DlPaint().setColor(DlColor(0x9bcbcbc)));
        }
        canvas->Restore();
      }
      canvas->Restore();
      //drawTextFrame
    }
    canvas->Restore();

    canvas->Save();
    {
      canvas->ClipRRect(
          SkRRect::MakeOval(SkRect::MakeLTRB(201.25, 10, 361.25, 170)),
          DlCanvas::ClipOp::kIntersect, true);
      SkRect save_layer_bounds = SkRect::MakeLTRB(201.25, 10, 361.25, 170);
      DlMatrixImageFilter backdrop(
          SkMatrix::MakeAll(3, 0, -299.25, 0, 3, -949, 0, 0, 1),
          DlImageSampling::kLinear);
      canvas->SaveLayer(&save_layer_bounds, /*paint=*/nullptr, &backdrop);
      {
        canvas->Translate(201.25, 10);
        auto paint = DlPaint()
                         .setAntiAlias(true)
                         .setColor(DlColor(0xff2196f3))
                         .setStrokeWidth(5)
                         .setDrawStyle(DlDrawStyle::kStroke);
        canvas->DrawCircle(SkPoint::Make(80, 80), 80, paint);
        paint.setColor(DlColor(0xfff44336));
        paint.setStrokeWidth(1.666666626930237);
        canvas->DrawRect(SkRect::MakeLTRB(70, 70, 90, 90), paint);
        paint.setColor(DlColor(0xff000000));
        canvas->DrawLine(SkPoint::Make(75.19999694824219, 80),
                         SkPoint::Make(84.80000305175781, 80), paint);
        canvas->DrawLine(SkPoint::Make(80, 75.19999694824219),
                         SkPoint::Make(80, 84.80000305175781), paint);
      }
      canvas->Restore();
    }
    canvas->Restore();
  };

  DisplayListBuilder builder;
  std::vector<sk_sp<DlImage>> images;
  images.emplace_back(CreateDlImageForFixture("kalimba.jpg"));
  draw(&builder, images);

  ASSERT_TRUE(OpenPlaygroundHere(builder.Build()));
}

}  // namespace testing
}  // namespace flutter
