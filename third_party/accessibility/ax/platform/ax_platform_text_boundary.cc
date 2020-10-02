// Copyright 2019 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ax_platform_text_boundary.h"

#include "../ax_enums.h"

namespace ax {

// #if BUILDFLAG(USE_ATK)
// ax::TextBoundary FromAtkTextBoundary(AtkTextBoundary boundary) {
//   // These are listed in order of their definition in the ATK header.
//   switch (boundary) {
//     case ATK_TEXT_BOUNDARY_CHAR:
//       return ax::TextBoundary::kCharacter;
//     case ATK_TEXT_BOUNDARY_WORD_START:
//       return ax::TextBoundary::kWordStart;
//     case ATK_TEXT_BOUNDARY_WORD_END:
//       return ax::TextBoundary::kWordEnd;
//     case ATK_TEXT_BOUNDARY_SENTENCE_START:
//       return ax::TextBoundary::kSentenceStart;
//     case ATK_TEXT_BOUNDARY_SENTENCE_END:
//       return ax::TextBoundary::kSentenceEnd;
//     case ATK_TEXT_BOUNDARY_LINE_START:
//       return ax::TextBoundary::kLineStart;
//     case ATK_TEXT_BOUNDARY_LINE_END:
//       return ax::TextBoundary::kLineEnd;
//   }
// }

// #if ATK_CHECK_VERSION(2, 10, 0)
// ax::TextBoundary FromAtkTextGranularity(AtkTextGranularity granularity) {
//   // These are listed in order of their definition in the ATK header.
//   switch (granularity) {
//     case ATK_TEXT_GRANULARITY_CHAR:
//       return ax::TextBoundary::kCharacter;
//     case ATK_TEXT_GRANULARITY_WORD:
//       return ax::TextBoundary::kWordStart;
//     case ATK_TEXT_GRANULARITY_SENTENCE:
//       return ax::TextBoundary::kSentenceStart;
//     case ATK_TEXT_GRANULARITY_LINE:
//       return ax::TextBoundary::kLineStart;
//     case ATK_TEXT_GRANULARITY_PARAGRAPH:
//       return ax::TextBoundary::kParagraphStart;
//   }
// }
// #endif  // ATK_CHECK_VERSION(2, 10, 0)
// #endif  // BUILDFLAG(USE_ATK)

#ifdef OS_WIN
ax::TextBoundary FromIA2TextBoundary(IA2TextBoundaryType boundary) {
  switch (boundary) {
    case IA2_TEXT_BOUNDARY_CHAR:
      return ax::TextBoundary::kCharacter;
    case IA2_TEXT_BOUNDARY_WORD:
      return ax::TextBoundary::kWordStart;
    case IA2_TEXT_BOUNDARY_LINE:
      return ax::TextBoundary::kLineStart;
    case IA2_TEXT_BOUNDARY_SENTENCE:
      return ax::TextBoundary::kSentenceStart;
    case IA2_TEXT_BOUNDARY_PARAGRAPH:
      return ax::TextBoundary::kParagraphStart;
    case IA2_TEXT_BOUNDARY_ALL:
      return ax::TextBoundary::kObject;
  }
}

ax::TextBoundary FromUIATextUnit(TextUnit unit) {
  // These are listed in order of their definition in the Microsoft
  // documentation.
  switch (unit) {
    case TextUnit_Character:
      return ax::TextBoundary::kCharacter;
    case TextUnit_Format:
      return ax::TextBoundary::kFormat;
    case TextUnit_Word:
      return ax::TextBoundary::kWordStart;
    case TextUnit_Line:
      return ax::TextBoundary::kLineStart;
    case TextUnit_Paragraph:
      return ax::TextBoundary::kParagraphStart;
    case TextUnit_Page:
      // UI Automation's TextUnit_Page cannot be reliably supported in a Web
      // document. We return kWebPage which is the next best thing.
    case TextUnit_Document:
      return ax::TextBoundary::kWebPage;
  }
}
#endif  // OS_WIN

}  // namespace ax
