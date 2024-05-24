// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_LIB_WEB_UI_SKWASM_SKIA_STRATEGY_H_
#define FLUTTER_LIB_WEB_UI_SKWASM_SKIA_STRATEGY_H_

#include "third_party/skia/include/core/SkBBHFactory.h"
#include "third_party/skia/include/core/SkCanvas.h"
#include "third_party/skia/include/core/SkColorFilter.h"
#include "third_party/skia/include/core/SkColorSpace.h"
#include "third_party/skia/include/core/SkContourMeasure.h"
#include "third_party/skia/include/core/SkData.h"
#include "third_party/skia/include/core/SkFontMgr.h"
#include "third_party/skia/include/core/SkImage.h"
#include "third_party/skia/include/core/SkImageFilter.h"
#include "third_party/skia/include/core/SkImageInfo.h"
#include "third_party/skia/include/core/SkMaskFilter.h"
#include "third_party/skia/include/core/SkMatrix.h"
#include "third_party/skia/include/core/SkPaint.h"
#include "third_party/skia/include/core/SkPath.h"
#include "third_party/skia/include/core/SkPicture.h"
#include "third_party/skia/include/core/SkPictureRecorder.h"
#include "third_party/skia/include/core/SkPoint3.h"
#include "third_party/skia/include/core/SkRRect.h"
#include "third_party/skia/include/core/SkShader.h"
#include "third_party/skia/include/core/SkString.h"
#include "third_party/skia/include/core/SkSurface.h"
#include "third_party/skia/include/core/SkVertices.h"
#include "third_party/skia/include/core/SkSamplingOptions.h"
#include "third_party/skia/include/encode/SkPngEncoder.h"
#include "third_party/skia/include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "third_party/skia/include/gpu/ganesh/gl/GrGLDirectContext.h"
#include "third_party/skia/include/gpu/GpuTypes.h"
#include "third_party/skia/include/gpu/GrBackendSurface.h"
#include "third_party/skia/include/gpu/GrDirectContext.h"
#include "third_party/skia/include/gpu/ganesh/GrExternalTextureGenerator.h"
#include "third_party/skia/include/gpu/ganesh/SkImageGanesh.h"
#include "third_party/skia/include/gpu/ganesh/SkSurfaceGanesh.h"
#include "third_party/skia/include/gpu/ganesh/gl/GrGLBackendSurface.h"
#include "third_party/skia/include/gpu/gl/GrGLInterface.h"
#include "third_party/skia/include/gpu/gl/GrGLTypes.h"
#include "third_party/skia/include/effects/SkGradientShader.h"
#include "third_party/skia/include/effects/SkImageFilters.h"
#include "third_party/skia/include/effects/SkRuntimeEffect.h"
#include "third_party/skia/include/pathops/SkPathOps.h"
#include "third_party/skia/include/ports/SkFontMgr_empty.h"
#include "third_party/skia/include/utils/SkShadowUtils.h"
#include "third_party/skia/modules/skparagraph/include/FontCollection.h"
#include "third_party/skia/modules/skparagraph/include/Paragraph.h"
#include "third_party/skia/modules/skparagraph/include/TypefaceFontProvider.h"

namespace Skwasm {
    using Scalar = SkScalar;
    using Point = SkPoint;
    using Point3 = SkPoint3;
    using Color = SkColor;
    using TileMode = SkTileMode;
    using Canvas = SkCanvas;
}

#endif  // FLUTTER_LIB_WEB_UI_SKWASM_SKIA_STRATEGY_H_
