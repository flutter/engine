// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/text/paragraph_impl_txt.h"
#include "flutter/common/threads.h"
#include "flutter/lib/ui/text/paragraph.h"
#include "flutter/lib/ui/text/paragraph_impl.h"
#include "lib/ftl/tasks/task_runner.h"

#include "lib/txt/src/paragraph_constraints.h"

namespace blink {

ParagraphImplTxt::ParagraphImplTxt(std::unique_ptr<txt::Paragraph>& paragraph) {
  m_paragraph = std::move(paragraph);
}

ParagraphImplTxt::~ParagraphImplTxt() {}

double ParagraphImplTxt::width() {
  return m_width;
}

double ParagraphImplTxt::height() {  // TODO.
  return m_paragraph->GetHeight();
}

double ParagraphImplTxt::minIntrinsicWidth() {  // TODO.
  return FLT_MAX;
}

double ParagraphImplTxt::maxIntrinsicWidth() {  // TODO.
  return FLT_MAX;
}

double ParagraphImplTxt::alphabeticBaseline() {  // TODO.
  return m_paragraph->GetAlphabeticBaseline();
}

double ParagraphImplTxt::ideographicBaseline() {  // TODO.
  return m_paragraph->GetIdeographicBaseline();
}

bool ParagraphImplTxt::didExceedMaxLines() {
  return m_paragraph->DidExceedMaxLines();
}

void ParagraphImplTxt::layout(double width) {
  m_width = width;
  m_paragraph->Layout(txt::ParagraphConstraints{width});
}

void ParagraphImplTxt::paint(Canvas* canvas, double x, double y) {
  SkCanvas* skCanvas = canvas->canvas();
  if (!skCanvas)
    return;
  txt::ParagraphStyle pStyle = m_paragraph->GetParagraphStyle();
  m_paragraph->Paint(skCanvas, x, y);
}

std::vector<TextBox> ParagraphImplTxt::getRectsForRange(unsigned start,
                                                        unsigned end) {
  return std::vector<TextBox>{0ull};
}

Dart_Handle ParagraphImplTxt::getPositionForOffset(double dx,
                                                   double dy) {  // TODO.
  return NULL;
}

Dart_Handle ParagraphImplTxt::getWordBoundary(unsigned offset) {  // TODO.
  return NULL;
}

}  // namespace blink
