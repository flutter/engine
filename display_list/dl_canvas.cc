// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter/display_list/dl_canvas.h"

#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkPoint3.h"
#include "third_party/skia/include/utils/SkShadowUtils.h"

namespace flutter {

DlFRect DlCanvas::ComputeShadowBounds(const DlPath& path,
                                      float elevation,
                                      DlScalar dpr,
                                      const DlTransform& ctm) {
  SkPath sk_path = path.GetSkiaPath();
  SkRect shadow_bounds(sk_path.getBounds());
  SkShadowUtils::GetLocalBounds(
      ctm.ToSkMatrix(), sk_path, SkPoint3::Make(0, 0, dpr * elevation),
      SkPoint3::Make(0, -1, 1), kShadowLightRadius / kShadowLightHeight,
      SkShadowFlags::kDirectionalLight_ShadowFlag, &shadow_bounds);
  return DlFRect::MakeBounds(shadow_bounds);
}

}  // namespace flutter
