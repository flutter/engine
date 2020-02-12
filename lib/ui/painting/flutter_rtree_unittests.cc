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

TEST_F(FlutterRTreeTest, DetectsIntersection) {
  // https://fiddle.skia.org/c/02b6328629ade6240dc4a830a4cc4cbb

  auto r_tree = sk_make_sp<FlutterRTree>();
  auto rtree_factory = FlutterRTreeFactory(r_tree);

  auto recorder = std::make_unique<SkPictureRecorder>();

  auto recording_window = SkRect::MakeIWH(1000, 1000);

  auto rect_paint = SkPaint();
  rect_paint.setColor(SkColors::kCyan);
  rect_paint.setStyle(SkPaint::Style::kFill_Style);

  SkCanvas* recording_canvas =
      recorder->beginRecording(recording_window, &rtree_factory);
  recording_canvas->drawRect(SkRect::MakeXYWH(20, 20, 20, 20), rect_paint);

  recording_canvas->save();
  recording_canvas->translate(-20, -20);
  recording_canvas->drawRect(SkRect::MakeXYWH(100, 100, 30, 30), rect_paint);
  recording_canvas->restore();

  recording_canvas->drawRect(SkRect::MakeXYWH(120, 120, 30, 30), rect_paint);

  recorder->finishRecordingAsPicture();

  auto hits = std::vector<SkRect*>();
  auto unobstructed = SkRect::MakeXYWH(50, 50, 20, 20);
  r_tree->searchRects(unobstructed, &hits);
  ASSERT_TRUE(hits.empty());
  hits.clear();

  // Hits that partially overlap with a drawn area return bboxes describing the
  // intersection of the query and the drawn area.
  auto one_hit = SkRect::MakeXYWH(140, 140, 30, 30);
  r_tree->searchRects(one_hit, &hits);
  ASSERT_EQ(1UL, hits.size());
  ASSERT_EQ(*hits[0], SkRect::MakeXYWH(120, 120, 30, 30));
}

}  // namespace testing
}  // namespace flutter
