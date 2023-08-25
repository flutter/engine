// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include "display_list/dl_builder.h"
#include "display_list/dl_paint.h"
#include "flutter/fml/macros.h"
#include "gtest/gtest.h"
#include "skia/paragraph_builder_skia.h"
#include "skia/paragraph_skia.h"
#include "testing/canvas_test.h"
#include "testing/display_list_testing.h"
#include "third_party/skia/modules/skparagraph/include/ParagraphBuilder.h"
#include "txt/paragraph.h"

namespace flutter {
namespace testing {

template <typename T>
class PainterTestBase : public CanvasTestBase<T> {
 public:
  PainterTestBase() = default;

  void PretendImpellerIsEnabled(bool impeller) { impeller_ = impeller; }

  bool impeller_ = false;
};

using PainterTest = PainterTestBase<::testing::Test>;

TEST_F(PainterTest, RendersSolidLine) {
  // Test that, irrespective of backend, we render a solid line.
  PretendImpellerIsEnabled(false);

  auto p_style = txt::ParagraphStyle();
  auto f_collection = std::make_shared<txt::FontCollection>();
  f_collection->SetDefaultFontManager(0);
  auto pb_skia = txt::ParagraphBuilderSkia(p_style, f_collection, false);

  pb_skia.AddText(u"Hello, world!");

  auto t_style = txt::TextStyle();
  t_style.decoration_style = txt::TextDecorationStyle::kSolid;
  pb_skia.PushStyle(t_style);
  auto builder = flutter::DisplayListBuilder();
  pb_skia.Build()->Paint(&builder, 0, 0);

  DisplayListBuilder expected;
  expected.DrawLine(SkPoint::Make(0, 0), SkPoint::Make(0, 0),
                    DlPaint(DlColor::kBlack()));
  EXPECT_TRUE(DisplayListsEQ_Verbose(builder.Build(), expected.Build()));
}

}  // namespace testing
}  // namespace flutter
