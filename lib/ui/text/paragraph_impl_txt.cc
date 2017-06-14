// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/lib/ui/text/paragraph_impl_txt.h"
#include "flutter/lib/ui/text/paragraph.h"
#include "flutter/lib/ui/text/paragraph_impl.h"

#include "flutter/common/threads.h"
#include "flutter/sky/engine/platform/text/TextBoundaries.h"
#include "lib/ftl/tasks/task_runner.h"

#include "lib/txt/src/paragraph_constraints.h"

namespace blink {

ParagraphImplTxt::ParagraphImplTxt() {}

ParagraphImplTxt::~ParagraphImplTxt() {}

void ParagraphImplTxt::setRenderView(
    PassOwnPtr<RenderView> renderView,
    std::unique_ptr<txt::Paragraph>& paragraph) {
  tempblink.setRenderView(renderView, paragraph);
  m_paragraph = std::move(paragraph);
}

double ParagraphImplTxt::width() {
  return m_width;
}

double ParagraphImplTxt::height() {
  return m_paragraph
      ->GetHeight();  // tempblink.height();  // m_paragraph->GetHeight();
}

double ParagraphImplTxt::minIntrinsicWidth() {  // TODO.
  return tempblink.minIntrinsicWidth();
}

double ParagraphImplTxt::maxIntrinsicWidth() {  // TODO.
  return tempblink.maxIntrinsicWidth();
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

int ParagraphImplTxt::absoluteOffsetForPosition(
    const PositionWithAffinity& position) {
  return 0;
}

Dart_Handle ParagraphImplTxt::getPositionForOffset(double dx,
                                                   double dy) {  // TODO.
  return tempblink.getPositionForOffset(dx, dy);
}

Dart_Handle ParagraphImplTxt::getWordBoundary(unsigned offset) {  // TODO.
  return tempblink.getWordBoundary(offset);
}

}  // namespace blink
