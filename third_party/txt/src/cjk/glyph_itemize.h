#ifndef FLUTTER_TXT_CJK_GLYPH_ITEMIZE_H
#define FLUTTER_TXT_CJK_GLYPH_ITEMIZE_H

#include <include/core/SkFont.h>
#include <memory>
#include <vector>

#include "glyph_run.h"
#include "minikin/FontCollection.h"
#include "minikin/Layout.h"
#include "minikin/MinikinFont.h"
#include "script_run.h"
#include "txt/styled_runs.h"

namespace txt {

#define SHAPE_METHOD(name)                                             \
  void get_glyph_run_##name(                                           \
      const std::vector<uint16_t>& text, const ScriptRun& run,         \
      const minikin::MinikinFont* minikin_font,                        \
      const minikin::FontStyle& minikin_style,                         \
      const minikin::MinikinPaint& minikin_paint,                      \
      const std::shared_ptr<minikin::FontCollection>& font_collection, \
      minikin::Layout& layout, SkFont& sk_font,                        \
      std::vector<std::unique_ptr<GlyphRun>>& glyph_runs,              \
      bool is_space_standalone)

SHAPE_METHOD(skfont_space);
SHAPE_METHOD(hb_font);
SHAPE_METHOD(cmap);
SHAPE_METHOD(hb_complex);
SHAPE_METHOD(minikin_complex);

}  // namespace txt

#endif  // FLUTTER_TXT_CJK_GLYPH_ITEMIZE_H
