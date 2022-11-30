#ifndef FLUTTER_TXT_CJK_SCRIPT_RUN_H
#define FLUTTER_TXT_CJK_SCRIPT_RUN_H

#include "txt/text_style.h"

namespace txt {

enum class ScriptRunType {
  kCJKIdeograph,
  kHardbreak,
  kSpace,
  kComplex,
};

struct ScriptRun {
  const TextStyle& style;
  size_t start;  // start index in text, inclusive
  size_t end;    // end index in text, exclusive
  ScriptRunType type;
};

}  // namespace txt

#endif  // FLUTTER_TXT_CJK_SCRIPT_RUN_H
