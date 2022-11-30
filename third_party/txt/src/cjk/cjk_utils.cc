#include "cjk_utils.h"
#include "flutter/fml/logging.h"
#include "unicode/uchar.h"

#ifdef HAVE_CJK_MEASURE_TIME
txt::measure_time_t::measure_time_t(const char* label)
    : label(strdup(label)), start(clock_t::now()) {}

txt::measure_time_t::~measure_time_t() {
  auto stop = clock_t::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::microseconds>(stop - start)
          .count();
  FML_DLOG(ERROR) << label << ": cost " << duration << "us";
  free((void*)label);
}
#endif

bool txt::is_space_separator(uint16_t code_unit) {
  // We don't use `u_isWhitespace(code_unit)` here, since ICU treat NBSP as not
  // whitespace.
  return code_unit == ' ' || code_unit == 0x00A0 || code_unit == 0x1680 ||
         (code_unit >= 0x2000 && code_unit <= 0x200A) || code_unit == 0x202F ||
         code_unit == 0x205F || code_unit == 0x3000;
}

bool txt::is_hard_break(uint16_t code_unit) {
  auto ulb = u_getIntPropertyValue(code_unit, UCHAR_LINE_BREAK);
  return ulb == U_LB_LINE_FEED || ulb == U_LB_MANDATORY_BREAK;
}

bool txt::is_fullwidth(uint16_t code_unit) {
  // code_unit U+FF00 is reserved
  return code_unit >= 0xFF00 && code_unit <= 0xFF60;
}

bool txt::is_cjk_ideographic_bmp(uint16_t code_unit) {
  // CJK Unified Ideographs, U+4E00 ~ U+9FFF
  if (code_unit >= 0x4E00 && code_unit <= 0x9FFF) {
    return true;
  }
  // CJK Unified Ideographs Extension A, U+3400 ~ U+4DBF
  if (code_unit >= 0x3400 && code_unit <= 0x4DBF) {
    return true;
  }
  // CJK Compatibility Ideographs: U+F900 ~ U+FAFF
  if (code_unit >= 0xF900 && code_unit <= 0xFAFF) {
    return true;
  }
  // CJK Symbols and Punctuation: U+3000 ~ U+303F
  if (code_unit >= 0x3000 && code_unit <= 0x303F) {
    return true;
  }
  return false;
}

uint32_t txt::next_u16_unicode(const uint16_t* chars,
                               size_t len,
                               size_t& iter) {
  const uint16_t v = chars[iter++];
  // test whether v in (0xd800..0xdfff), lead or trail surrogate
  if ((v & 0xf800) == 0xd800) {
    // test whether v in (0xd800..0xdbff), lead surrogate
    if (iter < len && (v & 0xfc00) == 0xd800) {
      const uint16_t v2 = chars[iter++];
      // test whether v2 in (0xdc00..0xdfff), trail surrogate
      if ((v2 & 0xfc00) == 0xdc00) {
        // (0xd800 0xdc00) in utf-16 maps to 0x10000 in ucs-32
        constexpr uint32_t delta = (0xd800 << 10) + 0xdc00 - 0x10000;
        return (((uint32_t)v) << 10) + v2 - delta;
      }
      iter -= 1;
      return 0xFFFDu;
    } else {
      return 0xFFFDu;
    }
  } else {
    return v;
  }
}
