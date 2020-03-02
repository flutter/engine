// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/painting/flutter_rtree.h"
#include "flutter/testing/testing.h"
#include "flutter/testing/thread_test.h"

#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"

namespace flutter {
namespace testing {

using FlutterRTreeTest = ThreadTest;

TEST_F(FlutterRTreeTest, NoIntersection) {
  auto r_tree = sk_make_sp<FlutterRTree>();
  auto rtree_factory = FlutterRTreeFactory(r_tree);
  auto recorder = std::make_unique<SkPictureRecorder>();
  auto recording_canvas =
      recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

  auto rect_paint = SkPaint();
  rect_paint.setColor(SkColors::kCyan);
  rect_paint.setStyle(SkPaint::Style::kFill_Style);

  recording_canvas->drawRect(SkRect::MakeXYWH(20, 20, 20, 20), rect_paint);
  recorder->finishRecordingAsPicture();

  auto hits = std::vector<SkRect*>();
  auto unobstructed = SkRect::MakeXYWH(40, 40, 20, 20);

  r_tree->searchRects(unobstructed, &hits);
  ASSERT_TRUE(hits.empty());
}

TEST_F(FlutterRTreeTest, Intersection) {
  auto r_tree = sk_make_sp<FlutterRTree>();
  auto rtree_factory = FlutterRTreeFactory(r_tree);
  auto recorder = std::make_unique<SkPictureRecorder>();
  auto recording_canvas =
      recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

  auto rect_paint = SkPaint();
  rect_paint.setColor(SkColors::kCyan);
  rect_paint.setStyle(SkPaint::Style::kFill_Style);

  recording_canvas->drawRect(SkRect::MakeXYWH(120, 120, 40, 40), rect_paint);

  recorder->finishRecordingAsPicture();

  // Hits that partially overlap with a drawn area return bboxes describing the
  // intersection of the query and the drawn area.
  auto one_hit = SkRect::MakeXYWH(140, 140, 40, 40);
  auto hits = std::vector<SkRect*>();

  r_tree->searchRects(one_hit, &hits);
  ASSERT_EQ(1UL, hits.size());
  ASSERT_EQ(*hits[0], SkRect::MakeXYWH(120, 120, 40, 40));
}

TEST_F(FlutterRTreeTest, JoinRectsWhenIntersected) {
  auto r_tree = sk_make_sp<FlutterRTree>();
  auto rtree_factory = FlutterRTreeFactory(r_tree);
  auto recorder = std::make_unique<SkPictureRecorder>();
  auto recording_canvas =
      recorder->beginRecording(SkRect::MakeIWH(1000, 1000), &rtree_factory);

  auto rect_paint = SkPaint();
  rect_paint.setColor(SkColors::kCyan);
  rect_paint.setStyle(SkPaint::Style::kFill_Style);

  recording_canvas->drawRect(SkRect::MakeXYWH(120, 120, 40, 40), rect_paint);
  recording_canvas->drawRect(SkRect::MakeXYWH(140, 140, 40, 40), rect_paint);

  recorder->finishRecordingAsPicture();

  // Hits that partially overlap with a drawn area return bboxes describing the
  // intersection of the query and the drawn area.
  auto one_hit = SkRect::MakeXYWH(142, 142, 10, 10);
  auto hits = std::vector<SkRect*>();

  r_tree->searchRects(one_hit, &hits);
  ASSERT_EQ(1UL, hits.size());
  ASSERT_EQ(*hits[0], SkRect::MakeXYWH(120, 120, 60, 60));
}

}  // namespace testing
}  // namespace flutter
