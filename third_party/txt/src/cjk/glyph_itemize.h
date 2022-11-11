#ifndef FLUTTER_TXT_CJK_GLYPH_ITEMIZE_H
#define FLUTTER_TXT_CJK_GLYPH_ITEMIZE_H

#include <include/core/SkFont.h>
#include <memory>
#include <vector>

#include "glyph_run.h"
#include "minikin/MinikinFont.h"
#include "txt/styled_runs.h"

namespace txt {

enum class ScriptRunType {
  kCJKIdeograph,
  kHardbreak,
  kComplex,
};

struct ScriptRun {
  const TextStyle& style;
  size_t start;
  size_t end;
  ScriptRunType type;
};

#define ITEMIZE_METHOD(name)                                   \
  std::unique_ptr<GlyphRun> get_glyph_run_##name(              \
      const std::vector<uint16_t>& text, const ScriptRun& run, \
      const minikin::MinikinFont* minikin_font, SkFont& sk_font)

ITEMIZE_METHOD(skfont);
ITEMIZE_METHOD(hb);
ITEMIZE_METHOD(hb_font);
ITEMIZE_METHOD(cmap);

}  // namespace txt

#endif  // FLUTTER_TXT_CJK_GLYPH_ITEMIZE_H
