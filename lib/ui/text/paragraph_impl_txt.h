// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_TEXT_PARAGRAPH_IMPL_TXT_H_
#define FLUTTER_LIB_UI_TEXT_PARAGRAPH_IMPL_TXT_H_

#include "flutter/lib/ui/painting/canvas.h"
#include "flutter/lib/ui/text/paragraph_impl.h"
#include "flutter/lib/ui/text/paragraph_impl_blink.h"
#include "flutter/lib/ui/text/text_box.h"
#include "lib/txt/src/paragraph.h"

namespace blink {

class ParagraphImplTxt : public ParagraphImpl {
 public:
  ~ParagraphImplTxt();

  explicit ParagraphImplTxt(std::unique_ptr<txt::Paragraph> paragraph);

  double width();
  double height();
  double minIntrinsicWidth();
  double maxIntrinsicWidth();
  double alphabeticBaseline();
  double ideographicBaseline();
  bool didExceedMaxLines();

  void layout(double width);
  void paint(Canvas* canvas, double x, double y);

  std::vector<TextBox> getRectsForRange(unsigned start, unsigned end);
  Dart_Handle getPositionForOffset(double dx, double dy);
  Dart_Handle getWordBoundary(unsigned offset);

 private:
  std::unique_ptr<txt::Paragraph> m_paragraph;
  double m_width = -1.0;
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_TEXT_PARAGRAPH_IMPL_TXT_H_
