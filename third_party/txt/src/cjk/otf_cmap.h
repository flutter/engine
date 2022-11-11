#ifndef FLUTTER_OTF_CMAP_H
#define FLUTTER_OTF_CMAP_H

#include <include/core/SkTypeface.h>
#include <memory>
#include "flutter/fml/macros.h"
#include "jump_table.h"

namespace txt {

struct glyph_group_t {
  uint32_t start_code_point;  // inclusive
  uint32_t end_code_point;    // inclusive
  uint32_t start_glyph_id;

  int compare(uint32_t codepoint) const;

  inline uint32_t get_glyph_id(uint32_t codepoint) const {
    return start_glyph_id + (codepoint - start_code_point);
  }
};

struct key_glyph_group_t {
  inline uint32_t operator()(const glyph_group_t& g) const {
    return g.start_code_point;
  }
};

struct cmp_glyph_group_t {
  inline int operator()(const glyph_group_t& g, uint32_t codepoint) const {
    return g.compare(codepoint);
  }
};

using base_cmap_t =
    jump_table_t<glyph_group_t, key_glyph_group_t, cmp_glyph_group_t>;

class cmap_t : public base_cmap_t {
 public:
  explicit cmap_t(std::vector<glyph_group_t>&& groups)
      : base_cmap_t(std::move(groups)) {}

  uint16_t get(uint32_t codepoint) const {
    const int index = find_index(codepoint);
    return index < 0 ? 0 : (*this)[index].get_glyph_id(codepoint);
  }

  FML_DISALLOW_COPY_AND_ASSIGN(cmap_t);
};

class OtfCmap {
 public:
  explicit OtfCmap(sk_sp<SkTypeface> sk_typeface);

  ~OtfCmap() = default;

  bool empty() const;
  uint16_t getGlyphId(uint32_t codepoint) const;

 private:
  std::unique_ptr<cmap_t> cmap_;

  FML_DISALLOW_COPY_AND_ASSIGN(OtfCmap);
};

}  // namespace txt

#endif  // FLUTTER_OTF_CMAP_H
