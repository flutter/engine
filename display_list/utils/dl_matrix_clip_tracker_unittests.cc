// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/utils/dl_matrix_clip_tracker.h"
#include "gtest/gtest.h"

namespace flutter {
namespace testing {

TEST(DisplayListMatrixClipTracker, Constructor) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  const DlTransform transform = DlTransform::MakeScale(4, 4);
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(5, 5, 15, 15);

  DisplayListMatrixClipTracker tracker1(cull_rect, transform);

  ASSERT_EQ(tracker1.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker1.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker1.matrix(), transform);
}

TEST(DisplayListMatrixClipTracker, Constructor4x4) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  // clang-format off
  const DlTransform transform = DlTransform::MakeRowMajor(4, 0, 0.5, 0,
                                                          0, 4, 0.5, 0,
                                                          0, 0, 4.0, 0,
                                                          0, 0, 0.0, 1);
  // clang-format on
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(5, 5, 15, 15);

  DisplayListMatrixClipTracker tracker(cull_rect, transform);

  ASSERT_EQ(tracker.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker.matrix(), transform);
}

TEST(DisplayListMatrixClipTracker, TransformTo4x4) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  // clang-format off
  const DlTransform transform = DlTransform::MakeRowMajor(4, 0, 0.5, 0,
                                                          0, 4, 0.5, 0,
                                                          0, 0, 4.0, 0,
                                                          0, 0, 0.0, 1);
  // clang-format on
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(5, 5, 15, 15);

  DisplayListMatrixClipTracker tracker(cull_rect, DlTransform());

  tracker.transform(transform);
  ASSERT_EQ(tracker.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker.matrix(), transform);
}

TEST(DisplayListMatrixClipTracker, SetTo4x4) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  // clang-format off
  const DlTransform transform = DlTransform::MakeRowMajor(4, 0, 0.5, 0,
                                                          0, 4, 0.5, 0,
                                                          0, 0, 4.0, 0,
                                                          0, 0, 0.0, 1);
  // clang-format on
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(5, 5, 15, 15);

  DisplayListMatrixClipTracker tracker(cull_rect, DlTransform());

  tracker.setTransform(transform);
  ASSERT_EQ(tracker.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker.matrix(), transform);
}

TEST(DisplayListMatrixClipTracker, Translate) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  const DlTransform transform = DlTransform::MakeScale(4, 4);
  const DlTransform translated_transform =
      DlTransform::MakeConcat(transform, DlTransform::MakeTranslate(5, 1));
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(0, 4, 10, 14);

  DisplayListMatrixClipTracker tracker1(cull_rect, transform);
  tracker1.translate(5, 1);

  ASSERT_EQ(tracker1.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker1.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker1.matrix(), translated_transform);
}

TEST(DisplayListMatrixClipTracker, Scale) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  const DlTransform transform = DlTransform::MakeScale(4, 4);
  const DlTransform scaled_transform =
      DlTransform::MakeConcat(transform, DlTransform::MakeScale(5, 2.5));
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(1, 2, 3, 6);

  DisplayListMatrixClipTracker tracker1(cull_rect, transform);
  tracker1.scale(5, 2.5);

  ASSERT_EQ(tracker1.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker1.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker1.matrix(), scaled_transform);
}

TEST(DisplayListMatrixClipTracker, Skew) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  const DlTransform transform = DlTransform::MakeScale(4, 4);
  const DlTransform skewed_transform =
      DlTransform::MakeConcat(transform, DlTransform::MakeSkew(.25, 0));
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(1.25, 5, 13.75, 15);

  DisplayListMatrixClipTracker tracker1(cull_rect, transform);
  tracker1.skew(.25, 0);

  ASSERT_EQ(tracker1.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker1.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker1.matrix(), skewed_transform);
}

TEST(DisplayListMatrixClipTracker, Rotate) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  const DlTransform transform = DlTransform::MakeScale(4, 4);
  const DlTransform rotated_transform = DlTransform::MakeConcat(
      transform, DlTransform::MakeRotate(DlAngle::Degrees(90)));
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(5, -15, 15, -5);

  DisplayListMatrixClipTracker tracker1(cull_rect, transform);
  tracker1.rotate(DlAngle::Degrees(90));

  ASSERT_EQ(tracker1.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker1.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker1.matrix(), rotated_transform);
}

TEST(DisplayListMatrixClipTracker, Transform2DAffine) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  const DlTransform transform = DlTransform::MakeScale(4, 4);

  const DlTransform transformed_transform =
      DlTransform::MakeConcat(transform, DlTransform::MakeAffine2D(2, 0, 5,  //
                                                                   0, 2, 6));
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(0, -0.5, 5, 4.5);

  DisplayListMatrixClipTracker tracker1(cull_rect, transform);
  tracker1.transform2DAffine(2, 0, 5,  //
                             0, 2, 6);

  ASSERT_EQ(tracker1.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker1.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker1.matrix(), transformed_transform);
}

TEST(DisplayListMatrixClipTracker, TransformFullPerspectiveUsing3x3Matrix) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  const DlTransform transform = DlTransform::MakeScale(4, 4);

  const DlTransform transformed_transform =
      DlTransform::MakeConcat(transform, DlTransform::MakeAffine2D(2, 0, 5,  //
                                                                   0, 2, 6));
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(0, -0.5, 5, 4.5);

  DisplayListMatrixClipTracker tracker1(cull_rect, transform);
  tracker1.transformFullPerspective(2, 0, 0, 5,  //
                                    0, 2, 0, 6,  //
                                    0, 0, 1, 0,  //
                                    0, 0, 0, 1);

  ASSERT_EQ(tracker1.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker1.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker1.matrix(), transformed_transform);
}

TEST(DisplayListMatrixClipTracker, TransformFullPerspectiveUsing4x4Matrix) {
  const DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 60, 60);
  const DlTransform transform = DlTransform::MakeScale(4, 4);

  const DlTransform transformed_transform =
      DlTransform::MakeConcat(transform,                             //
                              DlTransform::MakeRowMajor(2, 0, 0, 5,  //
                                                        0, 2, 0, 6,  //
                                                        0, 0, 1, 7,  //
                                                        0, 0, 0, 1));
  const DlFRect local_cull_rect = DlFRect::MakeLTRB(0, -0.5, 5, 4.5);

  DisplayListMatrixClipTracker tracker1(cull_rect, transform);
  tracker1.transformFullPerspective(2, 0, 0, 5,  //
                                    0, 2, 0, 6,  //
                                    0, 0, 1, 7,  //
                                    0, 0, 0, 1);

  ASSERT_EQ(tracker1.device_cull_rect(), cull_rect);
  ASSERT_EQ(tracker1.local_cull_rect(), local_cull_rect);
  ASSERT_EQ(tracker1.matrix(), transformed_transform);
}

TEST(DisplayListMatrixClipTracker, ClipDifference) {
  DlFRect cull_rect = DlFRect::MakeLTRB(20, 20, 40, 40);

  auto non_reducing = [&cull_rect](const DlFRect& diff_rect,
                                   const std::string& label) {
    {
      DisplayListMatrixClipTracker tracker(cull_rect, DlTransform());
      tracker.clipRect(diff_rect, DlCanvas::ClipOp::kDifference, false);
      ASSERT_EQ(tracker.device_cull_rect(), cull_rect) << label;
    }
    {
      DisplayListMatrixClipTracker tracker(cull_rect, DlTransform());
      const DlFRRect diff_rrect = DlFRRect::MakeRect(diff_rect);
      tracker.clipRRect(diff_rrect, DlCanvas::ClipOp::kDifference, false);
      ASSERT_EQ(tracker.device_cull_rect(), cull_rect) << label << " (RRect)";
    }
    {
      DisplayListMatrixClipTracker tracker(cull_rect, DlTransform());
      const DlPath diff_path = DlPath().AddRect(diff_rect);
      tracker.clipPath(diff_path, DlCanvas::ClipOp::kDifference, false);
      ASSERT_EQ(tracker.device_cull_rect(), cull_rect) << label << " (RRect)";
    }
  };

  auto reducing = [&cull_rect](const DlFRect& diff_rect,
                               const DlFRect& result_rect,
                               const std::string& label) {
    ASSERT_TRUE(result_rect.IsEmpty() || cull_rect.Contains(result_rect));
    {
      DisplayListMatrixClipTracker tracker(cull_rect, DlTransform());
      tracker.clipRect(diff_rect, DlCanvas::ClipOp::kDifference, false);
      ASSERT_EQ(tracker.device_cull_rect(), result_rect) << label;
    }
    {
      DisplayListMatrixClipTracker tracker(cull_rect, DlTransform());
      const DlFRRect diff_rrect = DlFRRect::MakeRect(diff_rect);
      tracker.clipRRect(diff_rrect, DlCanvas::ClipOp::kDifference, false);
      ASSERT_EQ(tracker.device_cull_rect(), result_rect) << label << " (RRect)";
    }
    {
      DisplayListMatrixClipTracker tracker(cull_rect, DlTransform());
      const DlPath diff_path = DlPath().AddRect(diff_rect);
      tracker.clipPath(diff_path, DlCanvas::ClipOp::kDifference, false);
      ASSERT_EQ(tracker.device_cull_rect(), result_rect) << label << " (RRect)";
    }
  };

  // Skim the corners and edge
  non_reducing(DlFRect::MakeLTRB(10, 10, 20, 20), "outside UL corner");
  non_reducing(DlFRect::MakeLTRB(20, 10, 40, 20), "Above");
  non_reducing(DlFRect::MakeLTRB(40, 10, 50, 20), "outside UR corner");
  non_reducing(DlFRect::MakeLTRB(40, 20, 50, 40), "Right");
  non_reducing(DlFRect::MakeLTRB(40, 40, 50, 50), "outside LR corner");
  non_reducing(DlFRect::MakeLTRB(20, 40, 40, 50), "Below");
  non_reducing(DlFRect::MakeLTRB(10, 40, 20, 50), "outside LR corner");
  non_reducing(DlFRect::MakeLTRB(10, 20, 20, 40), "Left");

  // Overlap corners
  non_reducing(DlFRect::MakeLTRB(15, 15, 25, 25), "covering UL corner");
  non_reducing(DlFRect::MakeLTRB(35, 15, 45, 25), "covering UR corner");
  non_reducing(DlFRect::MakeLTRB(35, 35, 45, 45), "covering LR corner");
  non_reducing(DlFRect::MakeLTRB(15, 35, 25, 45), "covering LL corner");

  // Overlap edges, but not across an entire side
  non_reducing(DlFRect::MakeLTRB(20, 15, 39, 25), "Top edge left-biased");
  non_reducing(DlFRect::MakeLTRB(21, 15, 40, 25), "Top edge, right biased");
  non_reducing(DlFRect::MakeLTRB(35, 20, 45, 39), "Right edge, top-biased");
  non_reducing(DlFRect::MakeLTRB(35, 21, 45, 40), "Right edge, bottom-biased");
  non_reducing(DlFRect::MakeLTRB(20, 35, 39, 45), "Bottom edge, left-biased");
  non_reducing(DlFRect::MakeLTRB(21, 35, 40, 45), "Bottom edge, right-biased");
  non_reducing(DlFRect::MakeLTRB(15, 20, 25, 39), "Left edge, top-biased");
  non_reducing(DlFRect::MakeLTRB(15, 21, 25, 40), "Left edge, bottom-biased");

  // Slice all the way through the middle
  non_reducing(DlFRect::MakeLTRB(25, 15, 35, 45), "Vertical interior slice");
  non_reducing(DlFRect::MakeLTRB(15, 25, 45, 35), "Horizontal interior slice");

  // Slice off each edge
  reducing(DlFRect::MakeLTRB(20, 15, 40, 25),  //
           DlFRect::MakeLTRB(20, 25, 40, 40),  //
           "Slice off top");
  reducing(DlFRect::MakeLTRB(35, 20, 45, 40),  //
           DlFRect::MakeLTRB(20, 20, 35, 40),  //
           "Slice off right");
  reducing(DlFRect::MakeLTRB(20, 35, 40, 45),  //
           DlFRect::MakeLTRB(20, 20, 40, 35),  //
           "Slice off bottom");
  reducing(DlFRect::MakeLTRB(15, 20, 25, 40),  //
           DlFRect::MakeLTRB(25, 20, 40, 40),  //
           "Slice off left");

  // cull rect contains diff rect
  non_reducing(DlFRect::MakeLTRB(21, 21, 39, 39), "Contained, non-covering");

  // cull rect equals diff rect
  reducing(cull_rect, DlFRect(), "Perfectly covering");

  // diff rect contains cull rect
  reducing(DlFRect::MakeLTRB(15, 15, 45, 45), DlFRect(), "Smothering");
}

TEST(DisplayListMatrixClipTracker, ClipPathWithInvertFillType) {
  DlFRect cull_rect = DlFRect::MakeLTRB(0, 0, 100.0, 100.0);
  DisplayListMatrixClipTracker builder(cull_rect, DlTransform());
  DlPath clip = DlPath().AddCircle(10.2, 11.3, 2).AddCircle(20.4, 25.7, 2);
  clip.SetFillType(DlPath::FillType::kInverseWinding);
  builder.clipPath(clip, DlCanvas::ClipOp::kIntersect, false);

  ASSERT_EQ(builder.local_cull_rect(), cull_rect);
  ASSERT_EQ(builder.device_cull_rect(), cull_rect);
}

TEST(DisplayListMatrixClipTracker, DiffClipPathWithInvertFillType) {
  DlFRect cull_rect = DlFRect::MakeLTRB(0, 0, 100.0, 100.0);
  DisplayListMatrixClipTracker tracker(cull_rect, DlTransform());

  DlPath clip = DlPath().AddCircle(10.2, 11.3, 2).AddCircle(20.4, 25.7, 2);
  clip.SetFillType(DlPath::FillType::kInverseWinding);
  DlFRect clip_bounds = DlFRect::MakeLTRB(8.2, 9.3, 22.4, 27.7);
  tracker.clipPath(clip, DlCanvas::ClipOp::kDifference, false);

  ASSERT_EQ(tracker.local_cull_rect(), clip_bounds);
  ASSERT_EQ(tracker.device_cull_rect(), clip_bounds);
}

}  // namespace testing
}  // namespace flutter
