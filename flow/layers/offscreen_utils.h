// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_FLOW_LAYERS_OFFSCREEN_UTILS_H_
#define FLUTTER_FLOW_LAYERS_OFFSCREEN_UTILS_H_

#include "third_party/skia/include/core/SkImageEncoder.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkSerialProcs.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkSurfaceCharacterization.h"
#include "third_party/skia/include/utils/SkBase64.h"

namespace flutter {

/// Returns a buffer containing a snapshot of the surface.
///
/// If compressed is true the data is encoded as PNG.
sk_sp<SkData> GetRasterData(sk_sp<SkSurface> offscreen_surface,
                            bool compressed);

}  // namespace flutter

#endif  // FLUTTER_FLOW_LAYERS_OFFSCREEN_UTILS_H_
