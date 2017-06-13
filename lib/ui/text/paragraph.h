// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_UI_TEXT_PARAGRAPH_H_
#define FLUTTER_LIB_UI_TEXT_PARAGRAPH_H_

#include "flutter/lib/ui/painting/canvas.h"
#include "flutter/lib/ui/text/paragraph_builder.h"
#include "flutter/lib/ui/text/paragraph_impl.h"
#include "flutter/lib/ui/text/paragraph_impl_blink.h"
#include "flutter/lib/ui/text/paragraph_impl_txt.h"
#include "flutter/lib/ui/text/text_box.h"
#include "flutter/sky/engine/core/rendering/RenderView.h"
#include "lib/tonic/dart_wrappable.h"
#include "lib/txt/src/paragraph.h"

namespace tonic {
class DartLibraryNatives;
}  // namespace tonic

namespace blink {

class Paragraph : public ftl::RefCountedThreadSafe<Paragraph>,
                  public tonic::DartWrappable {
  DEFINE_WRAPPERTYPEINFO();
  FRIEND_MAKE_REF_COUNTED(Paragraph);

 public:
  static ftl::RefPtr<Paragraph> create(
      PassOwnPtr<RenderView> renderView,
      const std::unique_ptr<txt::Paragraph>& paragraph) {
    return ftl::MakeRefCounted<Paragraph>(renderView, paragraph);
  }

  ~Paragraph() override;

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

  RenderView* renderView() const { return m_renderView.get(); }

  virtual size_t GetAllocationSize() override;

  static void RegisterNatives(tonic::DartLibraryNatives* natives);

  void toggleTxt();  // Toggle Blink or libtxt backend.

 private:
  friend class ParagraphBuilder;

  std::unique_ptr<ParagraphImpl> m_paragraphImpl;
  static bool m_usingBlink;

  RenderBox* firstChildBox() const { return m_renderView->firstChildBox(); }

  int absoluteOffsetForPosition(const PositionWithAffinity& position);

  explicit Paragraph(PassOwnPtr<RenderView> renderView,
                     const std::unique_ptr<txt::Paragraph>& paragraph);

  OwnPtr<RenderView> m_renderView;
};

}  // namespace blink

#endif  // FLUTTER_LIB_UI_TEXT_PARAGRAPH_H_
