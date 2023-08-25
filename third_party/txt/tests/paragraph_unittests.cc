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
#include "txt/text_decoration.h"

namespace flutter {
namespace testing {

template <typename T>
class PainterTestBase : public CanvasTestBase<T> {
 public:
  PainterTestBase() = default;

  void PretendImpellerIsEnabled(bool impeller) { impeller_ = impeller; }

 protected:
  void AssertDrawsSolidLine() {
    auto t_style = makeDecoratedStyle(txt::TextDecorationStyle::kSolid);
    auto pb_skia = makeParagraphBuilder();
    pb_skia.PushStyle(t_style);
    pb_skia.AddText(u"Hello World!");
    pb_skia.Pop();

    auto builder = flutter::DisplayListBuilder();
    auto paragraph = pb_skia.Build();
    paragraph->Layout(10000);
    paragraph->Paint(&builder, 0, 0);

    DisplayListBuilder expected;
    expected.DrawLine(SkPoint::Make(0, 0), SkPoint::Make(0, 0),
                      DlPaint(DlColor::kBlack()));
    EXPECT_TRUE(DisplayListsEQ_Verbose(builder.Build(), expected.Build()));
  }

 private:
  txt::ParagraphBuilderSkia makeParagraphBuilder() {
    auto p_style = txt::ParagraphStyle();
    auto f_collection = std::make_shared<txt::FontCollection>();
    /* Doesn't appear to do anything.
    auto sk_f_collection = f_collection->CreateSktFontCollection();
    f_collection->SetDefaultFontManager(sk_f_collection->getFallbackManager());
    */
    return txt::ParagraphBuilderSkia(p_style, f_collection, impeller_);
  }

  txt::TextStyle makeDecoratedStyle(txt::TextDecorationStyle style) {
    auto t_style = txt::TextStyle();
    t_style.font_weight = txt::FontWeight::w400;  // normal
    t_style.font_size = 14;
    t_style.decoration_style = style;
    return t_style;
  }

  bool impeller_ = false;
};

using PainterTest = PainterTestBase<::testing::Test>;

TEST_F(PainterTest, DrawsSolidLineSkia) {
  AssertDrawsSolidLine();
}

/*
TEST_F(PainterTest, DrawsSolidLineImpeller) {
  PretendImpellerIsEnabled(true);
  AssertDrawsSolidLine();
}
*/

}  // namespace testing
}  // namespace flutter
