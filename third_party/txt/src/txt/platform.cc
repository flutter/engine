// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "txt/platform.h"

#include "fml/logging.h"

namespace txt {

std::vector<std::string> GetDefaultFontFamilies() {
  return {"Arial"};
}

sk_sp<SkFontMgr> GetDefaultFontManager(uint32_t font_initialization_data) {
  // TODO(b/305780908) Replace this with a singleton that depends on which
  // platform we are on and which SkFontMgr was compiled in.
  return SkFontMgr::RefDefault();
}

}  // namespace txt
