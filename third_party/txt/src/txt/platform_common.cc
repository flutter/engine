// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "txt/platform.h"

#include "fml/logging.h"
#include "third_party/skia/include/core/SkFontMgr.h"
#include "third_party/skia/include/core/SkRefCnt.h"
#include "third_party/skia/include/core/SkTypeface.h"
#include "third_party/skia/include/core/SkFont.h"
#include "third_party/skia/include/core/SkFontStyle.h"

namespace txt {

static sk_sp<SkTypeface> MakeTypefaceFromName(const char* name, SkFontStyle style) {
  sk_sp<SkFontMgr> fm = GetDefaultFontManager(0);
  FML_CHECK(fm);
  sk_sp<SkTypeface> face = fm->legacyMakeTypeface(name, style);
  return face;
}

static sk_sp<SkTypeface> DefaultTypeface() {
  sk_sp<SkTypeface> face = MakeTypefaceFromName(nullptr, SkFontStyle());
  if (face) {
    return face;
  }
  // Due to how SkTypeface::MakeDefault() used to work, many callers of this
  // depend on the returned SkTypeface being non-null.
  // TODO(kjlubick) replace this with SkTypeface::MakeEmpty()
  face = SkTypeface::MakeDefault();
  FML_CHECK(face);
  return face;
}

SkFont DefaultFont() {
  return SkFont(DefaultTypeface());
}

}  // namespace txt
